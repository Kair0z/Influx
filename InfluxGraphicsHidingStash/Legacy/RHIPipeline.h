#pragma once

#ifndef __GR_RHI_PIPELINE_H_
#define __GR_RHI_PIPELINE_H_

#include "InfluxGraphics/Common.h"

namespace Influx::Graphics
{
	/* Pipeline Stuff: */
	namespace Internal
	{
		struct BaseResourceBinding
		{
			virtual const ERHIResourceBindingType GetBindingType() const noexcept = 0;
			virtual const uint32_t GetBindingSpace() const noexcept = 0;
			virtual const ERHIShaderStageFlags GetShaderStageFlags() const noexcept = 0;

			virtual const uint32_t GetNum() const noexcept = 0;

			virtual ~BaseResourceBinding() = default;
		};
	}

	namespace PipelineLayout
	{
		template <ERHIResourceBindingType TBindingType, uint32_t TNum, uint32_t TBindingSpace, ERHIShaderStageFlags TShaderStageFlags = ERHIShaderStageFlags::Default>
		struct ResourceBinding final : public Internal::BaseResourceBinding
		{
			virtual const ERHIResourceBindingType GetBindingType() const noexcept override { return TBindingType; }
			virtual const uint32_t GetBindingSpace() const noexcept override { return TBindingSpace; }
			virtual const ERHIShaderStageFlags GetShaderStageFlags() const noexcept override { return TShaderStageFlags; }
			virtual const uint32_t GetNum() const noexcept override { return TNum; }
			virtual ~ResourceBinding() = default;
		};

		template <uint32_t TNum32BitFloats, uint32_t TBindingSpace, ERHIShaderStageFlags TShaderStageFlags = ERHIShaderStageFlags::Default>
		using ConstantsBinding = ResourceBinding<ERHIResourceBindingType::Constants, TNum32BitFloats, TBindingSpace, TShaderStageFlags>;

		template <uint32_t TNum, uint32_t TBindingSpace, ERHIShaderStageFlags TShaderStageFlags = ERHIShaderStageFlags::Default>
		using SRVBinding = ResourceBinding<ERHIResourceBindingType::SRV, TNum, TBindingSpace, TShaderStageFlags>;

		template <uint32_t TNum, uint32_t TBindingSpace, ERHIShaderStageFlags TShaderStageFlags = ERHIShaderStageFlags::Default>
		using UAVBinding = ResourceBinding<ERHIResourceBindingType::UAV, TNum, TBindingSpace, TShaderStageFlags>;

		template <uint32_t TNum, uint32_t TBindingSpace, ERHIShaderStageFlags TShaderStageFlags = ERHIShaderStageFlags::Default>
		using CBVBinding = ResourceBinding<ERHIResourceBindingType::CBV, TNum, TBindingSpace, TShaderStageFlags>;
	}

	/* Graphics Pipeline Layout */
	// Dx12: RootSignature
	// Vulkan: PipelineLayout
	struct RHIGraphicsPipelineLayoutBindings
	{
		RHIGraphicsPipelineLayoutBindings() = default;
		virtual ~RHIGraphicsPipelineLayoutBindings() = default;

		/* Template only allows a class to be constructed if it derives from Internal::BaseResourceBinding */
		template <class TBindingType, typename = std::enable_if<std::is_base_of<Internal::BaseResourceBinding, TBindingType>::value>::type>
		void AddBinding()
		{
			m_resourceBindings.push_back(std::make_shared<TBindingType>(TBindingType()));
		}

		Vector<SharedPtr<Internal::BaseResourceBinding>> m_resourceBindings{};
	};
	struct RHIGraphicsPipelineLayoutDescription
	{
		RHIGraphicsPipelineLayoutBindings LayoutBindings{};

		// Todo: Static Samplers
		// Todo: Flags
	};
	class RHIGraphicsPipelineLayout
	{
		friend class GraphicsAPI;

	public:
		RHIGraphicsPipelineLayout() = default;
		RHIGraphicsPipelineLayout(const RHIGraphicsPipelineLayout&) = delete;
		RHIGraphicsPipelineLayout(RHIGraphicsPipelineLayout&&) = delete;
		RHIGraphicsPipelineLayout& operator=(const RHIGraphicsPipelineLayout&) = delete;
		RHIGraphicsPipelineLayout& operator=(RHIGraphicsPipelineLayout&&) = delete;
		virtual ~RHIGraphicsPipelineLayout() = default;

	protected:
		RHIGraphicsPipelineLayoutDescription m_constructionDescription{};
	};

	/* Graphics Pipeline Object */
	struct RHIGraphicsPipelineDescription
	{
		// Input Layout

		// Primitive Topology
		ERHIPrimitiveTopologyType PrimitiveTopologyType;

		// RTV & DSV Info
		std::vector<ERHIFormat> RTVFormats;
		ERHIFormat DSVFormat = ERHIFormat::INVALID;

		// Depth
		bool bDepthEnabled = true;
		bool bStencilEnabled = false;

		// Rasterizer
		ERHICullMode RasterCullMode = ERHICullMode::BackFaceCull;
		ERHIFillMode RasterFillMode = ERHIFillMode::Solid;
		int RasterDepthBias = 0;									// Depth value added to a given pixel
		float RasterMaxDepthBias = 1;								// Max Depth bias value of a pixel
		bool bRasterDepthClipEnable = false;						// Specifies whether to enable clipping based on distance.
		bool bConservativeRaster = false;							// Conservative Rasterization means that all pixels that are at least partially covered by a rendered primitive are rasterized
		bool bAntialiasedLineEnable = false;						// Specifies whether to enable line antialiasing; only applies if doing line drawing

		// Shaders:
		// ...
		// ...
	};

	class RHIGraphicsPipeline
	{
	public:
		RHIGraphicsPipeline() = default;
		RHIGraphicsPipeline(const RHIGraphicsPipeline&) = delete;
		RHIGraphicsPipeline(RHIGraphicsPipeline&&) = delete;
		RHIGraphicsPipeline& operator=(const RHIGraphicsPipeline&) = delete;
		RHIGraphicsPipeline& operator=(RHIGraphicsPipeline&&) = delete;
		virtual ~RHIGraphicsPipeline() = default;

	protected:
		RHIGraphicsPipelineDescription m_constructionDescription;
		RHIGraphicsPipelineLayout* m_pipelinelayoutReference;
	};
}

#endif