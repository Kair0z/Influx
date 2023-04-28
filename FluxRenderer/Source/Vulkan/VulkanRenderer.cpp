#include "VulkanRenderer.h"

#include "InfluxGraphics/Vulkan/Vulkan.h"

#include "Core/Platform/WindowsPlatform.h"

// https://alain.xyz/blog/raw-vulkan

namespace Influx
{
	void VulkanRenderer::BuildRenderWork(Platform::WindowHandle windowHandle)
	{

	}

	void VulkanRenderer::PresentToWindow(Platform::WindowHandle windowHandle)
	{
		vk::PipelineStageFlags waitDstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
		vk::SubmitInfo submitInfo(1, &m_presentCompleteSemaphore, &waitDstStageMask,
			1, &m_commandBuffers[m_currentSwapchainBuffer], 1u,
			&m_renderCompleteSemaphore);

		m_commandQueue.submit(1u, &submitInfo, m_waitFences[m_currentSwapchainBuffer]);

		m_commandQueue.presentKHR
		(
			vk::PresentInfoKHR(
				1u,
				&m_renderCompleteSemaphore,
				1,
				&m_swapchain,
				&m_currentSwapchainBuffer,
				nullptr
			)
		);
	}

	void VulkanRenderer::WaitForPreviousFrame()
	{

	}

	void VulkanRenderer::Initialize()
	{
		InitializeDevice();
		InitializeCommandQueue();
		InitializePipeline();
		InitializeCommandList();
	}

	void VulkanRenderer::InitializeDevice()
	{
		vk::ApplicationInfo appInfo;
		appInfo = { .pApplicationName = "Renderer",
					.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
					.pEngineName = "RenderEngine l.o.l.",
					.engineVersion = VK_MAKE_VERSION(1, 0, 0),
					.apiVersion = VK_API_VERSION_1_2 };

		// Create the Vulkan instance
		m_instance = Graphics::Vulkan::CreateVkInstance(appInfo, 
			// wantedExtensions
			{
				VK_KHR_SURFACE_EXTENSION_NAME
				// VK_KHR_WIN32_SURFACE_EXTENSION_NAME
			},
			
			// wantedLayers
			{
#ifdef _DEBUG
				"VK_LAYER_LUNARG_standard_validation"
#endif
			} 
		);

		// Get the first available Physical device:
		m_physicalDevice = Graphics::Vulkan::GetAllVkPhysicalDevices(m_instance)[0u];

		// Create a logical device from the Physical Device:
		m_logicalDevice = Graphics::Vulkan::CreateVkLogicalDevice(m_physicalDevice,
			// queues to create
			{
				vk::DeviceQueueCreateInfo()
			},

			// wantedExtensions
			{
				VK_KHR_SWAPCHAIN_EXTENSION_NAME
			},

			// wantedLayers
			{
				//...
			}
		);
	}

	void VulkanRenderer::InitializeCommandQueue()
	{
		uint32 queueFamily = 0u;
		uint32 queueIndex = 0u;

		m_commandQueue = m_logicalDevice.getQueue(queueFamily, queueIndex);
	}

	void VulkanRenderer::InitializeDescriptorPools()
	{
		Vector<vk::DescriptorPoolSize> dpSizes =
		{
			{
				vk::DescriptorType::eUniformBuffer,
				1u
			}
		};

		vk::DescriptorPoolCreateInfo dpci({}, 1, dpSizes);
		m_descriptorPool = m_logicalDevice.createDescriptorPool(dpci);
	}

	void VulkanRenderer::InitializeCommandList()
	{
		uint32 queueFamily = 0u;
		vk::CommandPoolCreateInfo commandPoolInfo = vk::CommandPoolCreateInfo(
			vk::CommandPoolCreateFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer),
			queueFamily
		);

		m_commandPool = m_logicalDevice.createCommandPool(commandPoolInfo);

		// Lets allocate 1 command buffer for each swapchain image.
		Vector<vk::CommandBuffer> commandBuffers = m_logicalDevice.allocateCommandBuffers(
			vk::CommandBufferAllocateInfo(
				m_commandPool,
				vk::CommandBufferLevel::ePrimary,
				k_numSwapchainBuffers
			)
		);
	}

	void VulkanRenderer::InitializeSynchronization()
	{
		for (uint32 i = 0u; i < k_numSwapchainBuffers; ++i)
		{
			m_waitFences[i] = m_logicalDevice.createFence(vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled));
		}
	}

	void VulkanRenderer::InitializeRenderPass()
	{
		vk::Format surfaceColorFormat;
		vk::Format surfaceDepthFormat;

		Vector<vk::AttachmentDescription> attachmentDescriptions =
		{
			// Color attachment
			{
				vk::AttachmentDescriptionFlags(),
				surfaceColorFormat,
				vk::SampleCountFlagBits::e1,
				vk::AttachmentLoadOp::eClear,
				vk::AttachmentStoreOp::eStore,
				vk::AttachmentLoadOp::eDontCare,
				vk::AttachmentStoreOp::eDontCare,
				vk::ImageLayout::eUndefined,
				vk::ImageLayout::ePresentSrcKHR
			},

			// Depth Attachment
			{
				vk::AttachmentDescriptionFlags(),
				surfaceDepthFormat,
				vk::SampleCountFlagBits::e1,
				vk::AttachmentLoadOp::eClear,
				vk::AttachmentStoreOp::eDontCare,
				vk::AttachmentLoadOp::eDontCare,
				vk::AttachmentStoreOp::eDontCare,
				vk::ImageLayout::eUndefined,
				vk::ImageLayout::eDepthStencilAttachmentOptimal
			}
		};

		Vector<vk::AttachmentReference> colorReferences =
		{
			vk::AttachmentReference(0, vk::ImageLayout::eColorAttachmentOptimal)
		};

		Vector<vk::AttachmentReference> depthReferences =
		{
			vk::AttachmentReference(1, vk::ImageLayout::eDepthStencilAttachmentOptimal)
		};

		Vector<vk::SubpassDescription> subpasses =
		{
			{
				vk::SubpassDescriptionFlags(),
				vk::PipelineBindPoint::eGraphics,
				0,
				nullptr,
				static_cast<uint32>(colorReferences.size()),
				colorReferences.data(),
				nullptr,
				depthReferences.data(),
				0,
				nullptr
			}
		};

		Vector<vk::SubpassDependency> subDependencies =
		{
			{
				~0U,
				0,
				vk::PipelineStageFlagBits::eBottomOfPipe,
				vk::PipelineStageFlagBits::eColorAttachmentOutput,
				vk::AccessFlagBits::eMemoryRead,
				vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite,
				vk::DependencyFlagBits::eByRegion
			},
			{
				0,
				~0U,
				vk::PipelineStageFlagBits::eColorAttachmentOutput,
				vk::PipelineStageFlagBits::eBottomOfPipe,
				vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite,
				vk::AccessFlagBits::eMemoryRead,
				vk::DependencyFlagBits::eByRegion
			}
		};

		m_renderPass = m_logicalDevice.createRenderPass
		(
			{
				vk::RenderPassCreateFlags(),
				static_cast<uint32>(attachmentDescriptions.size()),
				attachmentDescriptions.data(),
				static_cast<uint32>(subpasses.size()),
				subpasses.data(),
				static_cast<uint32>(subDependencies.size()),
				subDependencies.data()
			}
		);
	}

	void VulkanRenderer::InitializeSwapchain(Platform::WindowHandle windowHandle)
	{
		const Math::Vectoru2& dim = Platform::GetClientWindowRect<uint32>(windowHandle).GetDimensions();

		m_surfaceSize = vk::Extent2D(dim.x, dim.y);

		// Get Vulkan Surface with CrossWindowGraphics
		// m_windowSurface = xgfx::getSurface(&window, instance);
		uint32 queueFamily = 0u;
		if (!m_physicalDevice.getSurfaceSupportKHR(queueFamily, m_windowSurface))
		{
			// Check if queueFamily supports this surface
			return;
		}

		// Get the swapchain images
		Vector<vk::Image> swapchainImages = m_logicalDevice.getSwapchainImagesKHR(m_swapchain);
		for (uint32 i = 0u; i < k_numSwapchainBuffers; ++i)
		{
			m_swapchainImages[i] = swapchainImages[i];
		}

		// Create Depth Image Data
		vk::Image depthImage = m_logicalDevice.createImage
		(
			{
				vk::ImageCreateFlags(),
				vk::ImageType::e2D,
				surfaceDepthFormat,
				vk::Extent3D(m_surfaceSize.width, m_surfaceSize.height, 1),
				1,
				1,
				vk::SampleCountFlagBits::e1,
				vk::ImageTiling::eOptimal,
				vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eTransferSrc,
				vk::SharingMode::eExclusive,
				queueFamilyIndices.size(),
				queueFamilyIndices.data(),
				vk::ImageLayout::eUndefined
			}
		);

		// All framebuffers / attachments will be the same size as the surface
		vk::SurfaceCapabilitiesKHR surfaceCapabilities 
			= m_physicalDevice.getSurfaceCapabilitiesKHR(m_windowSurface);
		
		if (!(surfaceCapabilities.currentExtent.width == -1 || surfaceCapabilities.currentExtent.height == -1)) 
		{
			m_surfaceSize	= surfaceCapabilities.currentExtent;
			m_renderArea	= vk::Rect2D(vk::Offset2D(), m_surfaceSize);
			m_viewport		= vk::Viewport(0.0f, 0.0f, 
				static_cast<float>(m_surfaceSize.width), 
				static_cast<float>(m_surfaceSize.height), 0, 1.0f);
		}
	}

	void VulkanRenderer::InitializePipeline()
	{
		
	}
}

