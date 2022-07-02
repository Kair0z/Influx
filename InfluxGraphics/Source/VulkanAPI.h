#pragma once

#ifndef _VULKAN_API_H_
#define _VULKAN_API_H_

#include "GraphicsAPI.h"

#include "Vulkan/vulkan.hpp"

namespace Influx::Graphics
{
	/* VulkanAPI -> RHI */
	class VulkanAPI final : public GraphicsAPI
	{
		/* Private constructor -> Singleton */
		VulkanAPI();
		virtual RHICommandQueue* CreateCommandQueue(const ECommandQueueType type) const override final;
		virtual RHISwapChain* CreateSwapChain(HWND windowHandle, RHICommandQueue* commandQueue) const override final;
		virtual RHIVertexBuffer* CreateVertexBuffer(float* initialData, UINT initialSizeInBytes, UINT initialStrideInBytes) const override final;
		virtual RHIConstantBuffer* CreateConstantBuffer(float* initialData, UINT initialSizeInBytes, UINT initialStrideInBytes) const override final;
		virtual RHITexture* CreateTexture(const RHITextureDescription& constructionArgs) const override final;

		virtual RHIRenderTargetView* CreateRenderTargetView(RHITexture* texture) const override final;
		virtual RHIRenderTargetView* CreateRenderTargetView(RHIResource* resource) const override final;
		virtual RHIConstantBufferView* CreateConstantBufferView(RHIResource* resource) const override final;
		virtual RHIUnorderedAccessView* CreateUnorderedAccessView(RHIResource* resource) const override final;
		virtual RHIShaderResourceView* CreateShaderResourceView(RHIResource* resource) const override final;
		virtual RHIDepthStencilView* CreateDepthStencilView(RHIResource* resource) const override final;

		virtual RHIGraphicsPipelineLayout* CreateGraphicsPipelineLayout(const RHIGraphicsPipelineLayoutDescription& constructionArgs) const override final;
		virtual RHIGraphicsPipeline* CreateGraphicsPipeline(const RHIGraphicsPipelineDescription& constructionArgs, RHIGraphicsPipelineLayout* pipelineLayoutReference) const override final;

		virtual RHIShader* CreateRHIShader(const std::vector<uint8_t>& fromCompiledData) const override final;
		virtual RHIShader* CreateRHIShader(const std::wstring& fromFilePath, const std::string& entryPoint, const std::string& target) const override final;
		virtual RHIShader* CreateRHIShader(const std::wstring& fromFilePath, const std::string& entryPoint, const ERHIShaderType shaderType, const ERHIShaderModel shaderModel = ERHIShaderModel::SM_5_0) const override final;

	public:
		/* Singleton Object holding references to ID3D12Device, IDXGIFactory, ... */
		static VulkanAPI& Get()
		{
			static VulkanAPI api{};
			return api;
		}
		virtual ~VulkanAPI();

	private:
		vk::Instance VkInstance;

	public:
		/* Vulkan Static creation functions */
		/* Provides inline static functions involving creating VulkanAPI Objects & Resources & General functionality */
		static void CreateInstance(vk::Instance& outResult, const std::string& appName = "None");

		static bool CheckValidationLayerSupport(const std::vector<const char*>& validationLayerNames);
	};
}

#endif