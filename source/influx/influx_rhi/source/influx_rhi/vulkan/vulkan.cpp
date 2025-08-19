#include "rhi_pch.h"
#include "influx_rhi.h"

#include "vulkan.hpp"

namespace influx::rhi
{
	using vk_physdevice	= VkPhysicalDevice;
	using vk_queue		= VkQueue;
	using vk_device		= VkDevice;
	using vk_buffer		= VkBuffer;
	using vk_image		= VkImage;

	template <typename _t, typename _p>
	inline result<_t*> cast(_p* ptr)
	{
		if (ptr == nullptr)
			return result<_t*>::make_error("cannot cast when ptr is nullptr!");

		_t* res = (_t*)ptr;
		if (res) return res;
		else return result<_t*>::make_error("failed casting ptr to type!");
	}

	result<object_native> create_native(const device_create_args& args)
	{
		using result_type = result<object_native>;
		auto physdevice = cast<vk_physdevice>(args.m_physdevice);
		if (!physdevice) return result_type::make_error("failed casting args.m_physdevice to vk_device");

		vk::DeviceCreateInfo desc{};
		vk_device vkdevice = vk::PhysicalDevice(*physdevice.get()).createDevice(desc, nullptr);
		return vkdevice;
	}
	result<object_native> create_native(const queue_create_args& args)
	{
		vk_queue vkqueue{};
		return vkqueue;
	}
	result<object_native> create_native(const swapchain_create_args& args);
	result<object_native> create_native(const descheap_create_args& args);
	result<object_native> create_native(const commandallocator_create_args& args);
	result<object_native> create_native(const commandlist_create_args& args);
	result<object_native> create_native(const fence_create_args& args);
	result<object_native> create_native(const buffer_create_args& args);
	result<object_native> create_native(const texture2D_create_args& args);
	result<object_native> create_native(const texture3D_create_args& args);
	result<object_native> create_native(const pipeline_create_args& args);
	result<object_native> create_native(const rootsignature_create_args& args);

	// [device]
	result<device> device::create(const device_create_args& args)
	{
		using result_type = result<device>;
		auto args_copy = args;

		// make instance
		{
			VkApplicationInfo app_info{};
			app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
			app_info.pApplicationName = args.m_app_name;
			app_info.applicationVersion = args.m_app_version;
			app_info.pEngineName = args.m_engine_name;
			app_info.engineVersion = args.m_engine_version;
			app_info.apiVersion = args.m_api_version;

			VkInstanceCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
			createInfo.pApplicationInfo = &app_info;
		}
		
		// if no physical device specified, create one...
		if (args.m_physdevice == nullptr)
		{
			vk_physdevice physdevice = vk::PhysicalDevice{};
		}

		// create the actual device
		auto native_create_res = create_native(args);
		if (!native_create_res)
			return result_type::make_error("failed creating native device!");

		device device{};
		device.m_create_args = args_copy;
		device.m_native_object = native_create_res.get();
		device.m_data = {};
		return device;
	}
	result<queue> device::create(const queue_create_args& args) const
	{
		using result_type = result<queue>;

		auto args_copy = args; args_copy.m_device = m_native_object;
		auto native = create_native(args_copy);
		if (!native)
			return result_type::make_error("failed creating native");

		queue queue{};
		queue.m_native_object = native.get();
		return queue;
	}
}