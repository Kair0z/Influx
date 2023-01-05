#pragma once

#ifndef __APP_APPRENDERER_H_
#define __APP_APPRENDERER_H_

#include "Renderer.h"

namespace Influx::Graphics
{
    class RHIDescriptorHeap;
    class RHICommandQueue;
    class RHISwapchain;
    class RHICommandList;
}

namespace Influx::Application
{
    class ApplicationRenderer final : public IRenderer
    {
    public:
        struct WindowInfo final
        {
            Platform::WindowHandle WindowHandle;
            Math::Vectoru2 WindowDimensions;
        };

        ApplicationRenderer(RHIDevicePtr device, const WindowInfo& windowInfo);

        virtual void OnRender() const override final;
        virtual ~ApplicationRenderer();

    private:
        const WindowInfo m_initialWindowInfo;

        Graphics::RHIDescriptorHeap* mp_rtvDescriptorHeap;
        Graphics::RHICommandQueue* mp_commandQueue;
        Graphics::RHISwapchain* mp_swapChain;
        Graphics::RHICommandList* mp_commandList;

        uint64_t m_frame;

    private:
        virtual void Initialize(const RHIDevicePtr) override final;
        virtual void Cleanup(const RHIDevicePtr) override final;
    };
}

#endif