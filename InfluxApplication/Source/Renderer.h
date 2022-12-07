#pragma once

#ifndef __APP_RENDERER_H_
#define __APP_RENDERER_H_

#include "Common.h"

constexpr uint8_t k_numFramesInFlight = 3u;
constexpr bool k_useWarp = true;
constexpr bool k_useVSync = true;

#if PLATFORM_WINDOWS
#include "../ImGui/imgui_impl_win32.h"
#endif

#if FLX_APP_RENDERER_D3D12
#include "../ImGui/imgui_impl_dx12.h"
#include "InfluxGraphics/D3D12/D3D12.h"

struct Dx12Renderer final
{
    struct FrameContext
    {
        ID3D12CommandAllocator* CommandAllocator;
        UINT64                  FenceValue;
    };

    uint64_t m_frame;

    ID3D12Device2* mp_dxDevice2;
    ID3D12DescriptorHeap* mp_dxRtvDescHeap;
    ID3D12DescriptorHeap* mp_dxSrvDescHeap;
    ID3D12CommandQueue* mp_dxCommandQueue;
    ID3D12GraphicsCommandList* mp_dxCommandList;
    ID3D12Fence* m_fence;
    uint64_t m_fenceLastSignaledValue;

    HANDLE m_fenceEvent;

    ID3D12Resource* mp_targetResources[k_numFramesInFlight];
    D3D12_CPU_DESCRIPTOR_HANDLE m_targetDescriptors[k_numFramesInFlight];
    FrameContext m_frameContexts[k_numFramesInFlight];

    IDXGISwapChain3* mp_dxgiSwapchain;
    HANDLE m_swapchainWaitableObject;
};

static Dx12Renderer g_renderer;
#endif

#endif