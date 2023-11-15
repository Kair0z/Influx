#include "renderer_pch.h"
#include "renderer.h"

#pragma comment (lib, "d3d12.lib")
#pragma comment (lib, "DXGI.lib")
#pragma comment (lib, "D3DCompiler.lib")

#include "foreign/ImGui/imgui_impl_dx12.h"

#include "core/platform/windows_platform.h"
#include "Core/Time.h"

#include "api/api.h"

#include <thread>

namespace influx::renderer
{
    // Helper function for acquiring the first available hardware adapter that supports Direct3D 12.
    // If no such adapter can be found, *ppAdapter will be set to nullptr.
    inline void get_hardware_adapter(IDXGIFactory1* pFactory, IDXGIAdapter1** ppAdapter, bool requestHighPerformanceAdapter)
    {
        *ppAdapter = nullptr;

        IDXGIAdapter1* adapter = nullptr;

        IDXGIFactory6* factory6;
        if (SUCCEEDED(pFactory->QueryInterface(IID_PPV_ARGS(&factory6))))
        {
            for (
                UINT adapterIndex = 0;
                SUCCEEDED(factory6->EnumAdapterByGpuPreference(
                    adapterIndex,
                    requestHighPerformanceAdapter == true ? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE : DXGI_GPU_PREFERENCE_UNSPECIFIED,
                    IID_PPV_ARGS(&adapter)));
                ++adapterIndex)
            {
                DXGI_ADAPTER_DESC1 desc;
                adapter->GetDesc1(&desc);

                if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
                {
                    // Don't select the Basic Render Driver adapter.
                    // If you want a software adapter, pass in "/warp" on the command line.
                    continue;
                }

                // Check to see whether the adapter supports Direct3D 12, but don't create the
                // actual device yet.
                if (SUCCEEDED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
                {
                    break;
                }
            }
        }

        if (adapter == nullptr)
        {
            for (UINT adapterIndex = 0; SUCCEEDED(pFactory->EnumAdapters1(adapterIndex, &adapter)); ++adapterIndex)
            {
                DXGI_ADAPTER_DESC1 desc;
                adapter->GetDesc1(&desc);

                if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
                {
                    // Don't select the Basic Render Driver adapter.
                    // If you want a software adapter, pass in "/warp" on the command line.
                    continue;
                }

                // Check to see whether the adapter supports Direct3D 12, but don't create the
                // actual device yet.
                if (SUCCEEDED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
                {
                    break;
                }
            }
        }

        *ppAdapter = adapter;
    }

    void renderer_state::initialize(const init_args& args)
    {
        UINT dxgiFactoryFlags = 0;

        // debug layer
#if defined(_DEBUG)
        // Enable the debug layer (requires the Graphics Tools "optional feature").
        // NOTE: Enabling the debug layer after device creation will invalidate the active device.
        {
            ID3D12Debug* debugController;
            if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
            {
                debugController->EnableDebugLayer();

                // Enable additional debug layers.
                dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
            }
        }
#endif
        // factory
        IDXGIFactory4* factory;
        CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory));
        mpdx_factory = factory;

        // device
        if (k_useWarp)
        {
            IDXGIAdapter* warpAdapter;
            factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter));

            D3D12CreateDevice(
                warpAdapter,
                D3D_FEATURE_LEVEL_11_0,
                IID_PPV_ARGS(&mpdx_device));
        }
        else
        {
            IDXGIAdapter1* hardwareAdapter;
            get_hardware_adapter(factory, &hardwareAdapter, true);

            D3D12CreateDevice(
                hardwareAdapter,
                D3D_FEATURE_LEVEL_11_0,
                IID_PPV_ARGS(&mpdx_device));
        }

        // Describe and create the command queue.
        D3D12_COMMAND_QUEUE_DESC queueDesc = {};
        queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        mpdx_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&mpdx_commandQueue));

        // create some commandlist & commandallocators
        mpdx_commandAllocators.resize(k_max_num_frames_in_flight);
        mpdx_commandLists.resize(k_max_num_frames_in_flight);
        for (uint8 i = 0u; i < k_max_num_frames_in_flight; ++i)
        {
            auto type = D3D12_COMMAND_LIST_TYPE_DIRECT;
            mpdx_device->CreateCommandAllocator(type, IID_PPV_ARGS(&mpdx_commandAllocators[i]));
            mpdx_device->CreateCommandList(0u, type, mpdx_commandAllocators[i], nullptr, IID_PPV_ARGS(&mpdx_commandLists[i]));
            ((ID3D12GraphicsCommandList*)mpdx_commandLists[i])->Close();
        }

        // create main fence
        mpdx_device->CreateFence(0u, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mpdx_fence));

        // describe and create the rtv heap
        {
            D3D12_DESCRIPTOR_HEAP_DESC desc_heap_desc{};
            desc_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
            desc_heap_desc.NumDescriptors = k_max_swapchain_buffers;
            desc_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
            mpdx_device->CreateDescriptorHeap(&desc_heap_desc, IID_PPV_ARGS(&mpdx_rtv_heap));
            m_rtvDescriptorSize = mpdx_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        }

        // create srv heap
        {
            D3D12_DESCRIPTOR_HEAP_DESC desc_heap_desc{};
            desc_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            desc_heap_desc.NumDescriptors = k_max_srvs;
            desc_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
            mpdx_device->CreateDescriptorHeap(&desc_heap_desc, IID_PPV_ARGS(&mpdx_srvheap));
            m_srvDescriptorSize = mpdx_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        }

        // Create the pipeline state, which includes compiling and loading shaders.
        {
            // Create a root signature consisting of a descriptor table with a single CBV.
            {
                D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
                // This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
                featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
                if (FAILED(mpdx_device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
                {
                    featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
                }

                // Allow input layout and deny uneccessary access to certain pipeline stages.
                D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
                    D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
                    D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
                    D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
                    D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
                // D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

                CD3DX12_ROOT_PARAMETER1 rootParameters[3]{};
                {
                    // view_constant_buffer root constants
                    rootParameters[0].InitAsConstants(sizeof(view_constant_buffer) / sizeof(float), 0u, 0u, D3D12_SHADER_VISIBILITY_VERTEX);
                    
                    // root cbv for material data
                    //rootParameters[1].InitAsConstantBufferView(0u, 0u);

                    // srv descriptor table for textures
                    CD3DX12_DESCRIPTOR_RANGE1 descriptor_ranges[1]{};
                    descriptor_ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1u, 0u, 0u);
                    rootParameters[1].InitAsDescriptorTable(_countof(descriptor_ranges), descriptor_ranges, D3D12_SHADER_VISIBILITY_PIXEL);
                }

                CD3DX12_STATIC_SAMPLER_DESC static_samplers[1]{};
                static_samplers[0].Init(0, D3D12_FILTER_COMPARISON_ANISOTROPIC);

                CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
                rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, _countof(static_samplers), static_samplers, rootSignatureFlags);

                ID3DBlob* signature;
                ID3DBlob* error;
                D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error);
                mpdx_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&mpdx_rootsignature));
            }

            ID3DBlob* vertexShader;
            ID3DBlob* pixelShader;

#if defined(_DEBUG)
            // Enable better shader debugging with the graphics debugging tools.
            UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
            UINT compileFlags = 0;
#endif
            wstring w_resource_dir = to_wstring(args.m_resource_dir);
            wstring shader_filepath = w_resource_dir + L"Shaders/shaders.hlsl";
            ::D3DCompileFromFile(shader_filepath.c_str(), nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &vertexShader, nullptr);
            ::D3DCompileFromFile(shader_filepath.c_str(), nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pixelShader, nullptr);

            // Define the vertex input layout.
            D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
            {
                { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,    D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
                { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,    D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
                { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,      D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
                { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0,       D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },

                // instance data
                { "INSTANCE_DATA", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0,                            D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 },
                { "INSTANCE_DATA", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 },
                { "INSTANCE_DATA", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 },
                { "INSTANCE_DATA", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 },

                // instance colour :) 
                { "INSTANCE_COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 2, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 }
            };

            // Describe and create the graphics pipeline state object (PSO).
            D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
            psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
            psoDesc.pRootSignature = mpdx_rootsignature;
            psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader);
            psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader);
            psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
            psoDesc.RasterizerState.FrontCounterClockwise = true;
            psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
            psoDesc.DepthStencilState.DepthEnable = FALSE;
            psoDesc.DepthStencilState.StencilEnable = FALSE;
            psoDesc.SampleMask = UINT_MAX;
            psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
            psoDesc.NumRenderTargets = 1;
            psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
            psoDesc.SampleDesc.Count = 1;
            mpdx_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&mpdx_pipeline));
        }

        m_is_initialized = true;
    }

    void renderer_state::initialize_imgui()
    {
        if (!is_initialized())
        {
            FLX_ASSERT(false);
            return;
        }

        // create an srv heap for the font
        D3D12_DESCRIPTOR_HEAP_DESC desc_heap_desc{};
        desc_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        desc_heap_desc.NumDescriptors = 1u;
        desc_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        mpdx_device->CreateDescriptorHeap(&desc_heap_desc, IID_PPV_ARGS(&mpdx_srvheap_imgui));

        ImGui_ImplDX12_Init(mpdx_device, k_max_num_frames_in_flight,
            DXGI_FORMAT_R8G8B8A8_UNORM, mpdx_srvheap_imgui,
            mpdx_srvheap_imgui->GetCPUDescriptorHandleForHeapStart(),
            mpdx_srvheap_imgui->GetGPUDescriptorHandleForHeapStart());

        // bit cheeky, this creates the imgui device objects.
        ImGui_ImplDX12_NewFrame();
    }

    void renderer_state::render_to_window(const scene_proxy* scene_proxy, const render_args& render_args, platform::window_handle window, const present_args& present)
    {
        if (mpdx_commandQueue == nullptr || !platform::is_window_valid(window))
        {
            return;
        }

        // recreate swapchain
        recreate_swapchain_from_window(k_default_buffering, window);

        // update instance buffers
        if (scene_proxy != nullptr)
        {
            update_instance_buffers(scene_proxy);
        }

        if (m_is_initialized_imgui)
        {
            ImGui_ImplDX12_NewFrame();
        }
        
        // open a new frame_context that's not in flight
        // this can stall us if the GPU hasn't finished working!
        per_frame_context new_frame_ctx{};
        {
            time::point start = time::get_now();
            new_frame_ctx = acquire_next_frame();
            new_frame_ctx.m_stats.m_ms_acquire = time::get_ms_between<float>(time::get_now(), start);
        }
        
        // record commandlist
        {
            time::point start = time::get_now();

            ID3D12GraphicsCommandList* cmdlist = new_frame_ctx.mpdx_commandList;
            ID3D12Resource* backbuffer = new_frame_ctx.mpdx_backbuffer;
            const D3D12_CPU_DESCRIPTOR_HANDLE& backbuffer_rtv = *new_frame_ctx.mpdx_rtv_handle;
            new_frame_ctx.mpdx_commandAllocator->Reset();
            cmdlist->Reset(new_frame_ctx.mpdx_commandAllocator, mpdx_pipeline);

            transition_resource(cmdlist, backbuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

            cmdlist->OMSetRenderTargets(1u, &backbuffer_rtv, false, nullptr);
            cmdlist->ClearRenderTargetView(backbuffer_rtv, reinterpret_cast<const FLOAT*>(&render_args.m_clear_colour.r), 0u, nullptr);

            if (scene_proxy != nullptr)
            {
                // scene general stuff
                update_view_constant_buffer(scene_proxy);
                cmdlist->SetGraphicsRootSignature(mpdx_rootsignature);
                cmdlist->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
                cmdlist->RSSetViewports(1, &m_viewport);
                cmdlist->RSSetScissorRects(1, &m_rect);
                cmdlist->SetGraphicsRoot32BitConstants(0u, sizeof(view_constant_buffer) / sizeof(float), &m_view_constant_buffer, 0u);

                // draw instanced
                for (auto pair : m_instance_map)
                {
                    if (pair.second.empty()) continue;

                    const uint32 num_instances = static_cast<uint32>(pair.second.size());
                    cmdlist->IASetVertexBuffers(0u, 1u, &m_meshdata_map[pair.first].mdx_vertexbuffer_view);
                    cmdlist->IASetVertexBuffers(1u, 1u, &m_meshdata_map[pair.first].mdx_instancebuffer_view);
                    cmdlist->IASetIndexBuffer(&m_meshdata_map[pair.first].mdx_indexbuffer_view);
                    cmdlist->DrawIndexedInstanced((uint32)m_meshdata_map[pair.first].m_data.m_indices.size(), num_instances, 0u, 0u, 0u);
                }
            }

            transition_resource(cmdlist, backbuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

            cmdlist->Close();
            new_frame_ctx.m_stats.m_ms_build = time::get_ms_between<float>(time::get_now(), start);
        }

        // submit to command queue
        submit_to_queue(new_frame_ctx);

        // present swapchain
        mpdx_swapchain->Present(present.m_vsync ? 1u : 0u, 0u);
    }

    void renderer_state::load(const string& title, const mesh_data& data)
    {
        // store in map:
        m_meshdata_map[title].m_data = data; // COPY

        mesh_data& my_data = m_meshdata_map[title].m_data;
        ID3D12Resource*& vertexbuffer_resource = m_meshdata_map[title].mp_vertexbuffer;
        ID3D12Resource*& indexbuffer_resource = m_meshdata_map[title].mp_indexbuffer;

        uint32 vertexbuffer_size = static_cast<uint32>(data.m_vertices.size() * sizeof(vertex_data));
        uint32 indexbuffer_size = static_cast<uint32>(data.m_indices.size() * sizeof(index));

        // (re)create buffers
        if (m_meshdata_map[title].m_indexbuffer_size < indexbuffer_size)
        {
            safe_release(indexbuffer_resource);

            D3D12_HEAP_PROPERTIES heap_props = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
            D3D12_RESOURCE_DESC resource_desc = CD3DX12_RESOURCE_DESC::Buffer(indexbuffer_size);
            mpdx_device->CreateCommittedResource(&heap_props, D3D12_HEAP_FLAG_NONE, &resource_desc,
                D3D12_RESOURCE_STATE_INDEX_BUFFER | D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&indexbuffer_resource));

            m_meshdata_map[title].m_indexbuffer_size = indexbuffer_size;
        }
        if (m_meshdata_map[title].m_vertexbuffer_size < vertexbuffer_size)
        {
            safe_release(vertexbuffer_resource);

            D3D12_HEAP_PROPERTIES heap_props = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
            D3D12_RESOURCE_DESC resource_desc = CD3DX12_RESOURCE_DESC::Buffer(vertexbuffer_size);
            mpdx_device->CreateCommittedResource(&heap_props, D3D12_HEAP_FLAG_NONE, &resource_desc,
                D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER | D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vertexbuffer_resource));

            m_meshdata_map[title].m_vertexbuffer_size = vertexbuffer_size;
        }

        // map data
        {
            // Copy the triangle data to the vertex buffer.
            UINT8* p_data_begin;
            CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
            vertexbuffer_resource->Map(0, &readRange, reinterpret_cast<void**>(&p_data_begin));
            memcpy(p_data_begin, my_data.m_vertices.data(), vertexbuffer_size);
            vertexbuffer_resource->Unmap(0, nullptr);
        }
        {
            // Copy the index data to the index buffer.
            UINT8* p_data_begin;
            CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
            indexbuffer_resource->Map(0, &readRange, reinterpret_cast<void**>(&p_data_begin));
            memcpy(p_data_begin, my_data.m_indices.data(), indexbuffer_size);
            indexbuffer_resource->Unmap(0, nullptr);
        }

        // ready views
        {
            m_meshdata_map[title].mdx_vertexbuffer_view.BufferLocation = vertexbuffer_resource->GetGPUVirtualAddress();
            m_meshdata_map[title].mdx_vertexbuffer_view.StrideInBytes = sizeof(vertex_data);
            m_meshdata_map[title].mdx_vertexbuffer_view.SizeInBytes = vertexbuffer_size;

            m_meshdata_map[title].mdx_indexbuffer_view.BufferLocation = indexbuffer_resource->GetGPUVirtualAddress();
            m_meshdata_map[title].mdx_indexbuffer_view.Format = DXGI_FORMAT_R32_UINT;
            m_meshdata_map[title].mdx_indexbuffer_view.SizeInBytes = indexbuffer_size;
        }
    }

    void renderer_state::load(const string& title, const texture_data& data)
    {
        // store in map:
        m_texturedata_map[title].m_data = data;

        texture_data& my_data = m_texturedata_map[title].m_data;
        ID3D12Resource*& my_resource = m_texturedata_map[title].mp_resource;
        uint32 resource_size = static_cast<uint32>(data.m_pixels.size() * sizeof(math::vectorf4));

        // recreate resource
        {
            safe_release(my_resource);

            DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
            UINT64 width = data.get_width();
            UINT height = data.get_height();
            UINT16 arraySize = 1;
            UINT16 mipLevels = 0;
            UINT sampleCount = 1;
            UINT sampleQuality = 0;
            D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE;
            D3D12_TEXTURE_LAYOUT layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
            UINT64 alignment = 0;

            D3D12_HEAP_PROPERTIES heap_props = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
            D3D12_RESOURCE_DESC resource_desc = CD3DX12_RESOURCE_DESC::Tex2D(format, width, height, arraySize, mipLevels, sampleCount, sampleQuality, flags, layout, alignment);
            mpdx_device->CreateCommittedResource(&heap_props, D3D12_HEAP_FLAG_NONE, &resource_desc,
                D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&my_resource));
        }

        // map data
        {
            UINT8* p_data_begin;
            CD3DX12_RANGE readRange(0, 0); // We do not intend to read from this resource on the CPU.
            my_resource->Map(0, &readRange, reinterpret_cast<void**>(&p_data_begin));
            memcpy(p_data_begin, my_data.m_pixels.data(), resource_size);
            my_resource->Unmap(0, nullptr);
        }

        // allocate handle
        {
            // cringe: first slot
            m_texturedata_map[title].m_srv_handle_cpu = mpdx_srvheap->GetCPUDescriptorHandleForHeapStart();
            m_texturedata_map[title].m_srv_handle_gpu = mpdx_srvheap->GetGPUDescriptorHandleForHeapStart();
        }
    }

    void renderer_state::load(const string& title, const material_data& data)
    {
        m_materialdata_map[title].m_data = data;
        material_data& my_data = m_materialdata_map[title].m_data;
        ID3D12Resource*& my_resource = m_materialdata_map[title].mp_resource;
        uint32 resource_size = sizeof(material_data);

        // recreate buffers
        {
            safe_release(my_resource);

            D3D12_HEAP_PROPERTIES heap_props = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
            D3D12_RESOURCE_DESC resource_desc = CD3DX12_RESOURCE_DESC::Buffer(resource_size);
            mpdx_device->CreateCommittedResource(&heap_props, D3D12_HEAP_FLAG_NONE, &resource_desc,
                D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&my_resource));
        }

        // map data
        {
            UINT8* p_data_begin;
            CD3DX12_RANGE readRange(0, 0); // We do not intend to read from this resource on the CPU.
            my_resource->Map(0, &readRange, reinterpret_cast<void**>(&p_data_begin));
            memcpy(p_data_begin, &my_data, resource_size);
            my_resource->Unmap(0, nullptr);
        }

        // allocate handle
        {
            // cringe: second slot
            m_materialdata_map[title].m_cbv_handle_cpu = D3D12_CPU_DESCRIPTOR_HANDLE(mpdx_srvheap->GetCPUDescriptorHandleForHeapStart().ptr + m_srvDescriptorSize);
            m_materialdata_map[title].m_cbv_handle_gpu = D3D12_GPU_DESCRIPTOR_HANDLE(mpdx_srvheap->GetGPUDescriptorHandleForHeapStart().ptr + m_srvDescriptorSize);
        }
    }

    const mesh_data* renderer_state::find_mesh_data(const string& title) const
    {
        if (m_meshdata_map.contains(title))
        {
            return &m_meshdata_map[title].m_data;
        }

        return nullptr;
    }

    vector<const mesh_data*> renderer_state::get_all_mesh_datas() const
    {
        vector<const mesh_data*> result{};
        for (auto pair : m_meshdata_map)
        {
            result.push_back(&pair.second.m_data);
        }
        return result;
    }

    void* renderer_state::get_backend_device() const
    {
        return reinterpret_cast<void*>(mpdx_device);
    }

    vector<frame_stats> renderer_state::get_frame_stats(const uint32 over_num_frames)
    {
        vector<frame_stats> stats{};

#ifdef min
#undef min
#endif

        uint32 num = std::min(over_num_frames, (uint32)m_frame_stats.size());
        stats.resize(num);

        for (uint32 i = 0u; i < num; ++i)
        {
            m_frame_stats.peak(i, stats[i]);
        }

        return stats;
    }

    bool renderer_state::is_initialized() const
    {
        return m_is_initialized;
    }

    bool renderer_state::is_initialized_imgui() const
    {
        return m_is_initialized_imgui;
    }

    void renderer_state::cleanup()
    {
        m_is_initialized = false;
    }

    void renderer_state::recreate_swapchain_from_window(const e_buffering& buffering, platform::window_handle handle)
    {
        math::rectu window_rect = platform::get_windowrect_client<uint32>(handle);
        swapchain_state new_state{};
        new_state.m_window_rect = window_rect;

        // viewport & scissor rect
        {
            m_viewport = CD3DX12_VIEWPORT{ 0.0f, 0.0f, static_cast<float>(window_rect.m_width_height.x), static_cast<float>(window_rect.m_width_height.y) };
            m_rect = CD3DX12_RECT{ 0, 0, static_cast<::LONG>(window_rect.m_width_height.x), static_cast<::LONG>(window_rect.m_width_height.y) };
            m_aspect_ratio = static_cast<float>(window_rect.get_aspect_ratio());
        }

        if (!is_swapchain_dirty(new_state))
        {
            return;
        }

        safe_release(mpdx_swapchain);

        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
        swapChainDesc.BufferCount = ::UINT(buffering);
        swapChainDesc.Width = ::UINT(window_rect.m_width_height.x);
        swapChainDesc.Height = ::UINT(window_rect.m_width_height.y);
        swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swapChainDesc.SampleDesc.Count = 1;

        IDXGISwapChain1* swapChain;
        mpdx_factory->CreateSwapChainForHwnd(
            mpdx_commandQueue,        // Swap chain needs the queue so that it can force a flush on it.
            (::HWND)handle,
            &swapChainDesc,
            nullptr,
            nullptr,
            &swapChain);

        // does not support fullscreen transitions.
        mpdx_factory->MakeWindowAssociation((::HWND)handle, DXGI_MWA_NO_ALT_ENTER);
        mpdx_swapchain = (IDXGISwapChain4*)swapChain;
        m_swapchain_buffer_idx = mpdx_swapchain->GetCurrentBackBufferIndex();

        // setup render targets:
        const uint8 num_buffers = static_cast<uint8>(buffering);
        mpdx_backbufferResources.resize(num_buffers);
        mpdx_backbuffer_rtvs.resize(num_buffers);
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtv_handle(mpdx_rtv_heap->GetCPUDescriptorHandleForHeapStart());
        for (uint8 i = 0u; i < num_buffers; ++i)
        {
            mpdx_swapchain->GetBuffer(i, IID_PPV_ARGS(&mpdx_backbufferResources[i]));
            mpdx_device->CreateRenderTargetView(mpdx_backbufferResources[i], nullptr, rtv_handle);
            mpdx_backbuffer_rtvs[i] = rtv_handle;
            rtv_handle.Offset(1u, m_rtvDescriptorSize);
        }

        // update state
        m_previous_swapchain_state = new_state;
    }

    void renderer_state::update_view_constant_buffer(const scene_proxy* proxy)
    {
        auto& camera = proxy->m_cameras[0u];
        auto view = math::matrix4x4f::make_view_RH(camera.m_position, camera.m_forward);
        auto proj = math::matrix4x4f::make_projection_RH(camera.m_fov, m_aspect_ratio, camera.m_near_plane, camera.m_far_plane);
        m_view_constant_buffer.m_wvp = view * proj;
    }

    void renderer_state::update_instance_buffers(const scene_proxy* proxy)
    {
        // remake our instance map
        m_instance_map.clear();
        for (const mesh_proxy& mesh : proxy->m_meshes)
        {
            if (m_meshdata_map.contains(mesh.m_name.c_str()))
            {
                m_instance_map[mesh.m_name].push_back({ mesh.m_transform, mesh.m_per_instance_colour });
            }
        }

        for (auto pair : m_instance_map)
        {
            if (pair.second.empty()) continue;
            if (!m_meshdata_map.contains(pair.first)) continue;

            const uint32 num_instances = static_cast<uint32>(pair.second.size());
            const uint32 new_buffer_size = num_instances * sizeof(instance_data);

            mesh_data_entry& entry = m_meshdata_map[pair.first];
            if (entry.m_instancebuffer_size < new_buffer_size)
            {
                // recreate buffer resource
                safe_release(entry.mp_instancebuffer);

                CD3DX12_RESOURCE_DESC instanceBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(new_buffer_size);

                // create resource on the uploadheap
                D3D12_HEAP_PROPERTIES upload_heap_props = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
                mpdx_device->CreateCommittedResource(
                    &upload_heap_props,
                    D3D12_HEAP_FLAG_NONE,
                    &instanceBufferDesc,
                    D3D12_RESOURCE_STATE_GENERIC_READ,
                    nullptr,
                    IID_PPV_ARGS(&entry.mp_instancebuffer));

                entry.m_instancebuffer_size = new_buffer_size;
            }

            // map data
            {
                UINT8* p_data_begin;
                CD3DX12_RANGE readRange(0, 0); // We do not intend to read from this resource on the CPU.
                entry.mp_instancebuffer->Map(0, &readRange, reinterpret_cast<void**>(&p_data_begin));
                memcpy(p_data_begin, pair.second.data(), new_buffer_size);
                entry.mp_instancebuffer->Unmap(0, nullptr);
            }

            entry.mdx_instancebuffer_view.BufferLocation = entry.mp_instancebuffer->GetGPUVirtualAddress();
            entry.mdx_instancebuffer_view.StrideInBytes = sizeof(instance_data);
            entry.mdx_instancebuffer_view.SizeInBytes = new_buffer_size;
        }
    }

    renderer_state::per_frame_context renderer_state::acquire_next_frame()
    {
        // wait for the oldest submitted frame to be signalled finished
        if (m_frames_in_flight.size() >= k_max_num_frames_in_flight)
        {
            auto& frame_to_wait = m_frames_in_flight.front();

            // wait for front of the queue finish
            ::WaitForSingleObject(frame_to_wait.m_complete_event, INFINITE);

            on_frame_finished(frame_to_wait);
            m_frames_in_flight.pop();
        }

        const uint8 frame_idx = m_frame % k_max_num_frames_in_flight;

        per_frame_context new_context{};
        new_context.m_frame = m_frame;
        new_context.mpdx_commandList = mpdx_commandLists[frame_idx];
        new_context.mpdx_commandAllocator = mpdx_commandAllocators[frame_idx];
        get_swapchain_buffer_and_rtv(m_frame, new_context.mpdx_backbuffer, new_context.mpdx_rtv_handle);
        return new_context;
    }

    void renderer_state::on_frame_finished(const per_frame_context& ctx)
    {
        frame_stats stats = ctx.m_stats;
        stats.m_ms_frame = time::get_ms_between<float>(time::get_now(), ctx.m_timepoint_created);
        m_frame_stats.push(stats);
    }

    void renderer_state::submit_to_queue(const per_frame_context& ctx)
    {
        m_frames_in_flight.push(ctx);

        ID3D12CommandList* gfx_cmdlists[] = { ctx.mpdx_commandList };
        mpdx_commandQueue->ExecuteCommandLists(_countof(gfx_cmdlists), gfx_cmdlists);
        mpdx_commandQueue->Signal(mpdx_fence, ctx.m_frame);
        mpdx_fence->SetEventOnCompletion(ctx.m_frame, (::HANDLE)ctx.m_complete_event);

        ++m_frame;
    }

    bool renderer_state::is_swapchain_dirty(const swapchain_state& new_swapchain) const
    {
        return m_previous_swapchain_state.m_window_rect != new_swapchain.m_window_rect;
    }

    bool renderer_state::get_swapchain_buffer_and_rtv(const frame_id for_frame, ID3D12Resource*& out_buffer, D3D12_CPU_DESCRIPTOR_HANDLE*& out_rtv)
    {
        uint8 frame_idx = for_frame % k_max_num_frames_in_flight;
        out_buffer = mpdx_backbufferResources[frame_idx];
        out_rtv = &mpdx_backbuffer_rtvs[frame_idx];
        return true;
    }

    void renderer_state::transition_resource(ID3D12GraphicsCommandList* cmdlist, ID3D12Resource* resource, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after)
    {
        CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(resource, before, after);
        cmdlist->ResourceBarrier(1u, &barrier);
    }

#pragma region frontend_api
    void initialize(const init_args& args)
    {
        renderer_state::get_instance().initialize(args);
    }

    void initialize_imgui()
    {
        renderer_state::get_instance().initialize_imgui();
    }

    void load(const string& title, const mesh_data& data)
    {
        renderer_state::get_instance().load(title, data);
    }

    void load(const string& title, const texture_data& data)
    {
        renderer_state::get_instance().load(title, data);
    }

    void load(const string& title, const material_data& data)
    {
        renderer_state::get_instance().load(title, data);
    }

    const mesh_data* find_mesh_data(const string& title)
    {
        return renderer_state::get_instance().find_mesh_data(title);
    }

    vector<const mesh_data*> get_all_mesh_datas()
    {
        return renderer_state::get_instance().get_all_mesh_datas();
    }

    void* get_backend_device()
    {
        return renderer_state::get_instance().get_backend_device();
    }

    void render_to_window(const scene_proxy* scene_proxy, const render_args& render_args, platform::window_handle window, const present_args& present)
    {
        renderer_state::get_instance().render_to_window(scene_proxy, render_args, window, present);
    }

    vector<frame_stats> get_frame_stats(const uint32 over_num_frames)
    {
        return renderer_state::get_instance().get_frame_stats(over_num_frames);
    }

    bool is_initialized()
    {
        return renderer_state::get_instance().is_initialized();
    }

     bool is_initialized_imgui()
    {
        return renderer_state::get_instance().is_initialized_imgui();
    }

    void cleanup()
    {
        renderer_state::get_instance().cleanup();
    }
#pragma endregion
}

