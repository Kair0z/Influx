#include "graphics_pch.h"
#include "influx_graphics/vulkan/vk_swapchain.h"
#include "influx_graphics/vulkan/vk_resource.h"
#include "influx_graphics/vulkan/vk_commandqueue.h"
#include "influx_graphics/vulkan/vk_resource_views.h"
#include "vk_headers.h"

#include "core/platform/win32/win32_window.h"

namespace influx::graphics
{
	vk_swapchain::vk_swapchain(
		const platform::window_handle& window, 
		const swapchain_desc& desc,
		const swapchain_dependencies& swapchain_dependencies, 
		const dependencies& vk_dependencies)
		: swapchain(desc, swapchain_dependencies)
	{
		// store the native commandqueue (present queue)
		m_vk_present_queue = *swapchain_dependencies.mp_command_queue->get_native<vk::Queue>();

		// format
		constexpr e_format k_format = e_format::rgba8;
		constexpr vk::Format k_vk_format = vk::Format::eR8G8B8A8Unorm;

		// create the window surface representation:
		vk::SurfaceKHR surface{};
		{
			VkSurfaceKHR temp_surface;
			VkWin32SurfaceCreateInfoKHR create_info{};
			create_info.hinstance = static_cast<::HINSTANCE>(platform::get_current_instance());
			create_info.hwnd = static_cast<::HWND>(window);
			create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;

			vkCreateWin32SurfaceKHR(vk_dependencies.m_vk_instance, &create_info, nullptr, &temp_surface);

			surface = temp_surface;
		}

		// create vk swapchain
		vk::SwapchainKHR vkswapchain{};
		vk::SurfaceFormatKHR surface_format = vk::SurfaceFormatKHR(k_vk_format, vk::ColorSpaceKHR::eSrgbNonlinear); // DEFAULT
		vk::Format swapchain_format = surface_format.format;
		{
			vk::SurfaceCapabilitiesKHR surface_capabilities = vk_dependencies.m_vk_gpu.getSurfaceCapabilitiesKHR(surface);
			vk::Extent2D swapchain_extent = surface_capabilities.currentExtent;
			vk::SurfaceTransformFlagBitsKHR pre_transform = surface_capabilities.currentTransform;

			// pick a composite alpha mode
			vk::CompositeAlphaFlagBitsKHR composite_alpha;
			{
				composite_alpha = (surface_capabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::ePreMultiplied) ? vk::CompositeAlphaFlagBitsKHR::ePreMultiplied
					: (surface_capabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::ePostMultiplied) ? vk::CompositeAlphaFlagBitsKHR::ePostMultiplied
					: (surface_capabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::eInherit) ? vk::CompositeAlphaFlagBitsKHR::eInherit
					: vk::CompositeAlphaFlagBitsKHR::eOpaque;
			}

			// pick a present mode
			vk::PresentModeKHR present_mode = vk::PresentModeKHR::eFifo;
			{
				for (const vk::PresentModeKHR& mode : vk_dependencies.m_vk_gpu.getSurfacePresentModesKHR(surface))
				{
					if (mode == vk::PresentModeKHR::eMailbox)
					{
						present_mode = mode;
						break;
					}

					if (mode == vk::PresentModeKHR::eImmediate)
					{
						present_mode = mode;
						break;
					}
				}
			}

			// NOTE: it's not guaranteed our graphics queue family index also supports present!
			vk::SwapchainCreateInfoKHR info
			{
				vk::SwapchainCreateFlagsKHR{},
				surface,
				desc.m_num_buffers,
				swapchain_format,
				surface_format.colorSpace,
				swapchain_extent,
				1u,
				vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc,
				vk::SharingMode::eExclusive,
				{},
				pre_transform,
				composite_alpha,
				present_mode,
				true,		// clipped
				nullptr		// oldswapchain
			};

			// actual create
			m_vk_swapchain = vk_dependencies.m_vk_device.createSwapchainKHR(info);
		}

		// get the images (resources)
		m_vk_images = vk_dependencies.m_vk_device.getSwapchainImagesKHR(m_vk_swapchain);

		// get the image views (rtvs)
		{
			vk::ImageViewCreateInfo info({}, {}, vk::ImageViewType::e2D, swapchain_format, {}, { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 });
			for (size_t i = 0u; i < m_vk_images.size(); ++i)
			{
				info.image = m_vk_images[i];
				m_vk_imageviews.push_back(vk_dependencies.m_vk_device.createImageView(info));
			}
		}

		resize(window);
	}

	void vk_swapchain::present(const present_args& args)
	{
		const uint32 buffer_index = get_current_backbuffer_index();
		auto result = m_vk_present_queue.presentKHR(vk::PresentInfoKHR({}, m_vk_swapchain, buffer_index));
	}

	uint8 vk_swapchain::acquire_backbuffer()
	{
		vk::ResultValue<uint32> current_index = m_vk_device.acquireNextImageKHR(m_vk_swapchain, {}/*timeout*/, {}/*semaphore*/, {});
		update_backbuffer_index(current_index.value);
		return current_index.value;
	}

	vector<resource*> vk_swapchain::create_resources()
	{
		vector<resource*> resources{};
		for (size_t i = 0u; i < m_vk_images.size(); ++i)
		{
			resources.push_back(new vk_resource(m_vk_images[i]));
		}
		return resources;
	}
}