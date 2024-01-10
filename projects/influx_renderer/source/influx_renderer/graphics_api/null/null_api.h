#include "renderer_pch.h"
#include "influx_renderer/graphics_api/graphics_api.h"

// null includes
// ...

namespace influx::renderer::api
{
    // global api manager
    class null_api final
        : public graphics_api
    {
    public:
        // gathers a list of physical devices (gpu's)
        inline vector<physical_device> get_physical_devices() override
        {
            return {};
        }

        // create a logical interface device based on the given physical device
        inline virtual logical_device create_logical_device(const physical_device& device) override
        {
            return logical_device();
        }

    private:
        vector<shared_ptr<base>> mp_children = {};
    };
}