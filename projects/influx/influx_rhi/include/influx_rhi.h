#pragma once

#include "core/pointer.h"
#include "core/container/vector.h"

#include "influx_rhi/rhi_common.h"
#include "influx_rhi/rhi_constants.h"
#include "influx_rhi/rhi_interface.h"

#if INFLUX_RHI_D3D12
#include "influx_rhi/d3d12/d3d12_headers.h"
#endif

#if INFLUX_RHI_VULKAN
#include "influx_rhi/vulkan/vulkan_headers.h"
#endif

namespace influx::rhi
{
    class queue : public wrapper<queue, ID3D12CommandQueue>
    {
    public:
        static shared_ptr<device> create(const device& dev);

    private:
        queue() = default;
    };

    class commandlist : public wrapper<commandlist, ID3D12GraphicsCommandlist>
    {
    public:
        static shared_ptr<commandlist> create(const device& dev);

    private:
        commandlist() = default;
    };

    class swapchain : public wrapper<swapchain, IDXGISwapChain4>
    {
    public:
        static shared_ptr<swapchain> create(const device& dev);

    private:
        swapchain() = default;
    };

    class device : public wrapper<device, ID3D12Device>
    {
    public:
        static shared_ptr<device> create(e_api_type);

        // expensive, involves destroying all current objects, and re-allocating new ones
        void        switch_api(e_api_type);
        e_api_type  get_api_type() const;

        shared_ptr<commandlist> create_commandlist();
        shared_ptr<queue>       create_queue();
        shared_ptr<swapchain>   create_swapchain();
        
    private:
        device() = default;
        vector<shared_ptr<wrapper>> m_children;
    };

    class allocator : public wrapper<allocator, ID3D12CommandAllocator>
    {
    public:

    };
}