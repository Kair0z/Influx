#pragma once

#ifndef _VULKAN_API_H_
#define _VULKAN_API_H_

#include "GraphicsAPI.h"

#include "Math/Math.h"

#pragma comment(lib, "vulkan-1.lib")

#if _WIN32
#define VK_USE_PLATFORM_WIN32_KHR 
#endif

#include "Vulkan/vulkan.hpp"

namespace Influx::Graphics
{
	/* VulkanAPI -> RHI */
	class VulkanAPI final : public GraphicsAPI
	{
		/* Private constructor -> Singleton */
		VulkanAPI();

		virtual RHICommandQueue* CreateCommandQueue(const ERHICommandQueueType type) const override final;
		virtual RHISwapChain* CreateSwapChain(HINSTANCE i, HWND windowHandle, RHICommandQueue* commandQueue) const override final;
		virtual RHIVertexBuffer* CreateVertexBuffer(float* initialData, UINT initialSizeInBytes, UINT initialStrideInBytes) const override final { return nullptr; }
		virtual RHIConstantBuffer* CreateConstantBuffer(float* initialData, UINT initialSizeInBytes, UINT initialStrideInBytes) const override final { return nullptr; }
		virtual RHITexture* CreateTexture(const RHITextureDescription& constructionArgs) const override final { return nullptr; }

		virtual RHIRenderTargetView* CreateRenderTargetView(RHITexture* texture) const override final { return nullptr; }
		virtual RHIRenderTargetView* CreateRenderTargetView(RHIResource* resource) const override final { return nullptr; }
		virtual RHIConstantBufferView* CreateConstantBufferView(RHIResource* resource) const override final { return nullptr; }
		virtual RHIUnorderedAccessView* CreateUnorderedAccessView(RHIResource* resource) const override final { return nullptr; }
		virtual RHIShaderResourceView* CreateShaderResourceView(RHIResource* resource) const override final { return nullptr; }
		virtual RHIDepthStencilView* CreateDepthStencilView(RHIResource* resource) const override final { return nullptr; }

		virtual RHIDescriptorHeap* CreateDescriptorHeap(const ERHIDescriptorType type, uint32_t numDescriptors, bool shaderVisible = false) const override final { return nullptr; }

		virtual RHIGraphicsPipelineLayout* CreateGraphicsPipelineLayout(const RHIGraphicsPipelineLayoutDescription& constructionArgs) const override final;
		virtual RHIGraphicsPipeline* CreateGraphicsPipeline(const RHIGraphicsPipelineDescription& constructionArgs, RHIGraphicsPipelineLayout* pipelineLayoutReference) const override final;
		virtual RHIGraphicsPipeline* CreateGraphicsPipeline(const RHIGraphicsPipelineDescription& constructionArgs, RHIGraphicsPipelineLayout* pipelineLayoutReference, RHIRenderPass* renderPass) const override final;

		virtual RHIShader* CreateRHIShader(const std::vector<uint8_t>& fromCompiledData) const override final;
		virtual RHIShader* CreateRHIShader(const std::wstring& fromFilePath, const std::string& entryPoint, const std::string& target) const override final { return nullptr; }
		virtual RHIShader* CreateRHIShader(const std::wstring& fromFilePath, const std::string& entryPoint, const ERHIShaderType shaderType, const ERHIShaderModel shaderModel = ERHIShaderModel::SM_5_0) const override final { return nullptr; }

		virtual RHIRenderPass* CreateRenderPass() const override final;
		virtual RHIRenderPass* CreateRenderPass(const std::vector<RHIRenderPassAttachmentDesc>& attachments,
			const std::vector<RHIRenderSubPassDesc>& subpasses, const std::vector<RHIRenderSubPassDependency>& dependencies) const override final;
	
	public:
		/* Singleton Object holding references to ID3D12Device, IDXGIFactory, ... */
		static VulkanAPI& Get()
		{
			static VulkanAPI api{};
			return api;
		}
		virtual ~VulkanAPI();

		vk::Instance GetInstance() { return VulkInstance; }

	private:
		// Wrapper for a physical device, its data & its created logical device
		struct VulkanDevice final
		{
			VulkanDevice() = default;
			VulkanDevice(vk::PhysicalDevice physicalDevice);
			virtual ~VulkanDevice();

			void CreateLogicalDevice(vk::PhysicalDeviceFeatures enabledFeatures, std::vector<const char*> enabledExtensions, bool useSwapChain = true, vk::QueueFlags requestedQueueTypes = vk::QueueFlagBits::eGraphics | vk::QueueFlagBits::eCompute);
			uint32_t GetQueueFamilyIndex(vk::QueueFlags  queueFlags) const;

			vk::Queue RequestDeviceQueue(vk::QueueFlagBits queueType, uint32_t queueIndex = 0) const;

			vk::PhysicalDevice VkPhysicalDevice;
			vk::Device VkLogicalDevice;

			vk::PhysicalDeviceProperties VkProperties;
			vk::PhysicalDeviceMemoryProperties VkMemoryProperties;
			vk::PhysicalDeviceFeatures VkFeatures;
			vk::PhysicalDeviceFeatures VkEnabledFeatures;
			std::vector<vk::QueueFamilyProperties> VkQueueFamilyProperties;

			struct
			{
				uint32_t Graphics;
				uint32_t Compute;
				uint32_t Transfer;
			} QueueFamilyIndices;
		};

	public:
		const VulkanDevice& GetDevice();
		const vk::Device& GetLogicalDevice();
		const vk::PhysicalDevice& GetPhysicalDevice();

		const VulkanDevice& GetDevice() const;
		const vk::Device& GetLogicalDevice() const;
		const vk::PhysicalDevice& GetPhysicalDevice() const;

	private:
		vk::Instance VulkInstance;
		VulkanDevice MainDevice;

		vk::PhysicalDevice PickFirstSuitablePhysicalDevice(const std::vector<vk::PhysicalDevice>& devices);

		vk::DebugUtilsMessengerEXT VkDebugMessenger;

	public:
		

		/* Vulkan Static creation functions */
		/* Provides inline static functions involving creating VulkanAPI Objects & Resources & General functionality */
		static void CreateInstance(vk::Instance& outResult, bool validation, const std::string& appName = "None");

		static std::vector<vk::PhysicalDevice> GetPhysicalDevices(const vk::Instance& instance);
		
		static vk::Device CreateLogicalDeviceAndQueues(vk::PhysicalDevice physicalDevice, std::vector<uint32_t> queueFamilyIndices, std::vector<vk::Queue>& outQueues);

		static bool CheckValidationLayerSupport(const std::vector<const char*>& validationLayerNames);

		static vk::CommandPool CreateCommandPool(const vk::Device& device, uint32_t queueFamilyIndex);

		struct SwapChainSupportDetails
		{
			vk::SurfaceCapabilitiesKHR VkSurfaceCapabilities;
			std::vector<vk::SurfaceFormatKHR> VkFormats;
			std::vector<vk::PresentModeKHR> VkPresentModes;
		};

		static SwapChainSupportDetails QuerySwapChainSupport(const vk::PhysicalDevice& physicalDevice, const vk::SurfaceKHR& surface);

		static vk::ImageView CreateImageView(const vk::Device& device, vk::Image image, vk::Format imageFormat, vk::ImageViewType viewType = vk::ImageViewType::e2D);

		static vk::DebugUtilsMessengerEXT SetupDebugMessenger(const vk::Instance& instance);

		static vk::Framebuffer CreateFrameBuffer(const vk::Device& device, const Math::Vector2u& dimensions, const vk::RenderPass& renderPass, const std::vector<vk::ImageView>& attachments)
		{
			vk::FramebufferCreateInfo createInfo{};
			createInfo.sType = vk::StructureType::eFramebufferCreateInfo;
			createInfo.renderPass = renderPass;
			createInfo.attachmentCount = (uint32_t)attachments.size();
			createInfo.pAttachments = attachments.data();
			createInfo.width = dimensions.x;
			createInfo.height = dimensions.y;
			createInfo.layers = 1;

			return device.createFramebuffer(createInfo, nullptr);
		}

		static vk::RenderPass CreateRenderPass(const vk::Device& device, const std::vector<vk::AttachmentDescription>& attachments, 
			const std::vector<vk::SubpassDescription>& subpasses, const std::vector<vk::SubpassDependency>& subpassDependencies)
		{
			vk::RenderPassCreateInfo createInfo{};
			createInfo.sType = vk::StructureType::eRenderPassCreateInfo;
			createInfo.attachmentCount = (uint32_t)attachments.size();
			createInfo.pAttachments = attachments.data();
			createInfo.subpassCount = (uint32_t)subpasses.size();
			createInfo.pSubpasses = subpasses.data();
			createInfo.dependencyCount = (uint32_t)subpassDependencies.size();
			createInfo.pDependencies = subpassDependencies.data();

			device.createRenderPass(createInfo, nullptr);
		}

	private:
		static VKAPI_ATTR VkBool32 VKAPI_CALL DebugMessageFunc(VkDebugUtilsMessageSeverityFlagBitsEXT       messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT              messageTypes,
			VkDebugUtilsMessengerCallbackDataEXT const* pCallbackData,
			void* /*pUserData*/);
	};

	/* VulkanCommandList */
	class VulkanCommandList final : public RHICommandList
	{
	public:
		/* RHICommandList API: */
		virtual void RecordRenderPass(RHIRenderPass* renderPass, const RHIRenderPassBeginInfo& beginInfo, Function<void(RHICommandList* cmdList)>) override final;
		virtual void TransitionResource(RHIResource* resource, const ERHIResourceState newState) override final {}
		virtual void ClearRTV(RHIRenderTargetView* renderTargetView, const Math::Vector4f& clearValue) override final {}
		virtual void BindScissorRect(const RHIScissorRect& scissorRect) override final {}
		virtual void BindViewports(const RHIViewport& viewport) override final {}
		virtual void BindVertexBuffer(RHIVertexBuffer* vertexBuffer) override final {}
		virtual void SetPrimitiveTopology(ERHIPrimitiveTopology topology) override final {}
		virtual void CopyResource(RHIResource* source, RHIResource* dest, bool forceTransition) override final {}
		virtual void ClearTextureAsRTV(RHITexture* texture, bool forceTransition) override final;
		virtual void ClearTextureAsRTV(RHITexture* texture, const Math::Vector4f& clearValue, bool forceTransition) override final;
		virtual void BindPipelineLayout(RHIGraphicsPipelineLayout* pipelineLayout) override final {}
		virtual void BindPipelineState(RHIGraphicsPipeline* pipeline) override final {}
		virtual void BindRenderTarget(RHIRenderTargetView* renderTargetView) override final {}
		virtual void DrawInstanced(uint32_t numVerticesPerInstance, uint32_t numInstances, uint32_t startVertexLocation, uint32_t startInstanceLocation) override final {}
		virtual void BindDescriptorheap(RHIDescriptorHeap* descriptorHeap) override final {}

		vk::CommandBuffer GetVulkanCommandBuffer() const;

	private:
		vk::CommandBuffer VkCommandBuffer;
		
		VulkanCommandList() = default;
		friend class VulkanCommandQueue;
	};

	/* VulkanCommandQueue */
	class VulkanCommandQueue final : public RHICommandQueue
	{
	public:
		virtual RHICommandList* SetupNewCommandList(GraphicsAPI* api) override final;
		virtual void ExecuteCommmandList(RHICommandList* commandList) override final {}
		virtual void Flush() override final {}

		vk::Queue GetVulkanQueue() const;

		~VulkanCommandQueue();

	private:
		vk::Queue VkCommandQueue;
		vk::CommandPool VkCommandPool;

		VulkanCommandQueue() = default;
		friend class VulkanAPI;
	};

	/* VulkanSwapChain */
	class VulkanSwapChain final : public RHISwapChain
	{
	public:
		virtual void Present(RHICommandQueue* commandQueue, bool VSync) override final;
		virtual void Resize(GraphicsAPI* api, RHICommandQueue* commandQueue, UINT newSizeX, UINT newSizeY) override final;

		~VulkanSwapChain();

	private:
		vk::SurfaceKHR VkSurface;
		vk::SwapchainKHR VkSwapChain;

		std::vector<vk::Image> VkSwapChainImages;
		vk::Format VkImageFormat;
		vk::Extent2D VkExtent;

		std::vector<vk::ImageView> VkImageViews;

		VulkanSwapChain() = default;
		friend class VulkanAPI;
	};
}

#endif