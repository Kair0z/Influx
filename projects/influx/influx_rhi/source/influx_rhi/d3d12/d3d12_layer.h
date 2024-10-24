#pragma once
#include "core/container/vector.h"

namespace influx::rhi::dx12
{
    // device creation
    using factory2 = IDXGIFactory2;
    factory2* create_factory(uint32 flags);

    using adapter1 = IDXGIAdapter1;
    vector<adapter1*> get_hardware_adapters(factory2* factory);

    using device = ID3D12Device;
    device* create_device(adapter1* adapter);

    void set_debug_layer_enabled(bool);
    
    // resources
    using queue = ID3D12CommandQueue;
    queue* create_queue(device*, D3D12_COMMAND_LIST_TYPE, D3D12_COMMAND_QUEUE_PRIORITY);

    using swapchain = IDXGISwapChain4;
    swapchain* create_swapchain(factory2*, queue*, ::HWND, uint32 width, uint32 height, DXGI_FORMAT, uint32 num_buffers);

    using descriptor_heap = ID3D12DescriptorHeap;
    descriptor_heap* create_descriptor_heap(device*, D3D12_DESCRIPTOR_HEAP_TYPE, uint32 capacity, bool shader_vis);

    using command_allocator = ID3D12CommandAllocator;
    command_allocator* create_command_allocator(device*, D3D12_COMMAND_LIST_TYPE);

    using pipeline_graphics = ID3D12PipelineState;
    pipeline_graphics* create_pipeline();

    using commandlist_graphics = ID3D12GraphicsCommandList;
    commandlist_graphics* create_commandlist_graphics(device*, command_allocator*, pipeline_graphics* init_pipeline);

    using rootsignature = ID3D12RootSignature;
    rootsignature* create_rootsignature();

    using fence = ID3D12Fence;
    fence* create_fence(device*, uint32 init_value);

    struct heap_desc
    {
        D3D12_HEAP_TYPE m_type;

    };

    struct buffer_desc
    {
        uint32 m_bytesize;
        D3D12_RESOURCE_FLAGS m_flags;
        D3D12_RESOURCE_STATES m_init_state;
    };

    struct texture_desc
    {
        uint32 m_width;
        uint32 m_height;
        uint32 m_depth;
        uint32 m_arraysize;
        uint32 m_num_mips;
        uint32 m_num_samples;
        DXGI_FORMAT m_format;
        D3D12_RESOURCE_FLAGS m_flags;
        D3D12_RESOURCE_STATES m_init_state;
    };

    using resource = ID3D12Resource;
    resource* create_resource(device*, const heap_desc&, const buffer_desc&);
    resource* create_resource(device*, const heap_desc&, const texture_desc&);

    // descriptors
    using descriptor_cpu = D3D12_CPU_DESCRIPTOR_HANDLE;
    using descriptor_gpu = D3D12_GPU_DESCRIPTOR_HANDLE;
    void create_rtv(device*, resource*, descriptor_cpu, DXGI_FORMAT);
    void create_dsv(device*, resource*, descriptor_cpu, DXGI_FORMAT);
    void create_texture_srv(device*, resource*, const D3D12_SHADER_RESOURCE_VIEW_DESC&, descriptor_cpu);
    void create_buffer_srv(device*, resource*, const D3D12_SHADER_RESOURCE_VIEW_DESC&, descriptor_cpu);
    void create_sampler_view(device*);
    void get_descriptor_strides(uint64& rtv, uint64& dsv, uint64& srv, uint64& sampler);
}