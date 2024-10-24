#pragma once

#include "core/pointer.h"
#include "core/container/vector.h"

#include "influx_rhi/rhi_common.h"
#include "influx_rhi/rhi_constants.h"
#include "influx_rhi/rhi_interface.h"

namespace influx::rhi
{
    class device;
    
    class queue
    {
    public:
        static shared_ptr<device> create(const device& dev);

    private:
        queue() = default;
    };

    class commandlist
    {
    public:
        static shared_ptr<commandlist> create(const device& dev);

    private:
        commandlist() = default;
    };

    class allocator
    {
    public:

    };

    class swapchain
    {
    public:
        static shared_ptr<swapchain> create(const device& dev);

    private:
        swapchain() = default;
    };

    class device
    {
    public:
        static device* create(e_api_type api);
        virtual ~device() = default;

        // expensive, involves destroying all current objects, and re-allocating new ones
        void        switch_api(e_api_type);
        e_api_type  get_api_type() const;

        shared_ptr<commandlist> create_commandlist();
        shared_ptr<queue>       create_queue();
        shared_ptr<swapchain>   create_swapchain();
        
    private:
        device() = default;
    };
}