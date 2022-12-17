#pragma once

#ifndef __APP_RENDERER_H_
#define __APP_RENDERER_H_

#include "Common.h"

#if PLATFORM_WINDOWS 
#include "Core/Platform/WindowsPlatform.h"
#endif

#if PLATFORM_WINDOWS && FLX_APP_RENDERER_D3D12
#include "InfluxGraphics/D3D12/D3D12.h"
#endif

namespace Influx
{
    constexpr uint8_t k_numFramesInFlight = 3u;
    constexpr bool k_useWarp = true;
    constexpr bool k_useVSync = true;

#if PLATFORM_WINDOWS && FLX_APP_RENDERER_D3D12
    class ImGuiRendererDx12 final
    {
        struct ImGuiSettings final
        {
            enum class EStyle
            {
                Dark,
                Max
            };

            EStyle Style = EStyle::Dark;
            float FontScale = 1.0f;
            bool WindowRounding = true;
        };

        struct FrameContext final
        {
            ID3D12CommandAllocator* CommandAllocator;
            UINT64                  FenceValue;
        };

        uint64_t m_frame;

        IDXGIFactory4* mp_dxgiFactory4;
        IDXGIAdapter4* mp_currentAdapter4;
        ID3D12Device2* mp_dxDevice2;
        ID3D12DescriptorHeap* mp_dxRtvDescHeap;
        ID3D12DescriptorHeap* mp_dxSrvDescHeap;
        ID3D12CommandQueue* mp_dxCommandQueue;
        ID3D12GraphicsCommandList* mp_dxCommandList;
        ID3D12Fence* m_fence;
        uint64_t m_fenceLastSignaledValue;

        HANDLE m_fenceEvent;

        ID3D12Resource* mp_targetResources[k_numFramesInFlight];
        D3D12_CPU_DESCRIPTOR_HANDLE m_windowTargetDescriptors[k_numFramesInFlight];
        FrameContext m_frameContexts[k_numFramesInFlight];

        IDXGISwapChain3* mp_dxgiSwapchain;
        HANDLE m_swapchainWaitableObject;

        Math::Vectorf4 m_clearColour;

        ImGuiSettings m_currentImGuiSettings;

    public:
        ImGuiRendererDx12(const Math::Vectoru2& windowDimensions, ::HWND windowHandle);
        ImGuiRendererDx12(const ImGuiRendererDx12&) = delete;
        ImGuiRendererDx12(ImGuiRendererDx12&&) = delete;
        ImGuiRendererDx12& operator=(const ImGuiRendererDx12&) = delete;
        ImGuiRendererDx12& operator=(ImGuiRendererDx12&&) = delete;
        virtual ~ImGuiRendererDx12();

        void RenderFrame(Function<void()> internalRenderFunction);
        void InitializeWindowRenderTargets();

        void SetClearColour(const Math::Vectorf4& clearColour);
        const Math::Vectorf4& GetClearColour() const;

    private:
        void InitializeImGui(::HWND windowHandle);
        void InitializeDx12(const Math::Vectoru2& windowDimensions, ::HWND windowHandle);
        void DestroyDx12();
        void DestroyDx12RenderTargets();

        FrameContext* WaitForNextFrameResources();
        void WaitForLastSubmittedFrame();

        void UpdateImGuiIO();

    public:
        LRESULT CALLBACK WindowsProcedure(::HWND hWnd, ::UINT uMsg, ::WPARAM wParam, ::LPARAM lParam);
    };
#endif // PLATFORM_WINDOWS && FLX_APP_RENDERER_D3D12
}

#endif