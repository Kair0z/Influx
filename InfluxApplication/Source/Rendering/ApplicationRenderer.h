#pragma once

#ifndef __APP_RENDERER_H_
#define __APP_RENDERER_H_

#include "Renderer.h"

namespace Influx::Application
{
    class ApplicationRenderer final : public IRenderer
    {
        struct WindowInfo final
        {
            Platform::WindowHandle WindowHandle;
            Math::Vectoru2 WindowDimensions;
        };

        const WindowInfo m_initialWindowInfo;

        Graphics::RHIDescriptorHeap* mp_rtvDescriptorHeap;
        Graphics::RHICommandQueue* mp_commandQueue;
        Graphics::RHISwapchain* mp_swapChain;
        Graphics::RHICommandList* mp_commandList;

        uint64_t m_frame;

    public:
        ApplicationRenderer(RHIDevicePtr device, const WindowInfo& windowInfo);

        virtual void OnRender() const override final;
        virtual ~ApplicationRenderer() = default;

    private:
        virtual void Initialize(const RHIDevicePtr) override final;
        virtual void Cleanup(const RHIDevicePtr) override final;
    };
}

#endif