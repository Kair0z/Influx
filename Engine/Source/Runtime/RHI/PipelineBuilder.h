#pragma once

#include "Core/Geometry/Vertex.h"
#include "Core/Container/Vector.h"
#include "Core/Memory/Reference.h"

#include "RHITypes.h"

namespace Influx
{
#pragma region ResourceBindingInfo
	namespace Internal
	{
		struct BaseResourceBinding
		{
			virtual const ERHIResourceBindingType GetBindingType() const noexcept = 0;
			virtual const uint32_t GetBindingSpace() const noexcept = 0;
			virtual const ERHIShaderStageFlags GetShaderStageFlags() const noexcept = 0;

			virtual const uint32_t GetNum() const noexcept = 0;
		};
	}

	template <uint32_t Num, ERHIResourceBindingType BindingType, uint32_t BindingSpace, ERHIShaderStageFlags ShaderStageFlags = ERHIShaderStageFlags::Default>
	struct ResourceBinding final : Internal::BaseResourceBinding
	{
		inline virtual const ERHIResourceBindingType GetBindingType() const noexcept override { return BindingType; };
		inline virtual const uint32_t GetBindingSpace() const noexcept override { return BindingSpace; };
		inline virtual const ERHIShaderStageFlags GetShaderStageFlags() const noexcept override { return ShaderStageFlags; };
		inline virtual const uint32_t GetNum() const noexcept override { return Num; };
	};

	template <uint32_t Num32BitConstants, uint32_t BindingSpace, ERHIShaderStageFlags ShaderStageFlags = ERHIShaderStageFlags::Default>
	using ConstantBinding = ResourceBinding<Num32BitConstants, ERHIResourceBindingType::Constants, BindingSpace, ShaderStageFlags>;

	template <uint32_t Num32BitConstants, uint32_t BindingSpace, ERHIShaderStageFlags ShaderStageFlags = ERHIShaderStageFlags::Default>
	using CBVBinding = ResourceBinding<Num32BitConstants, ERHIResourceBindingType::CBV, BindingSpace, ShaderStageFlags>;

	template <uint32_t Num32BitConstants, uint32_t BindingSpace, ERHIShaderStageFlags ShaderStageFlags = ERHIShaderStageFlags::Default>
	using SRVBinding = ResourceBinding<Num32BitConstants, ERHIResourceBindingType::SRV, BindingSpace, ShaderStageFlags>;

	template <uint32_t Num32BitConstants, uint32_t BindingSpace, ERHIShaderStageFlags ShaderStageFlags = ERHIShaderStageFlags::Default>
	using UAVBinding = ResourceBinding<Num32BitConstants, ERHIResourceBindingType::UAV, BindingSpace, ShaderStageFlags>;
#pragma endregion

	// Information about the Pipeline:
	// * ResourceBindings						
	// * RTV / DSV info
	// * Static Samplers						[TODO]
	// * [GRAPHICS] Input Layout				[TODO]
	// * [GRAPHICS] PrimitiveTopologyType		[TODO]
	// * [GRAPHICS] Shaders						[TODO]
	struct GraphicsPipelineBuilder final
	{
	public:
		template <class BindingType, typename = std::enable_if<std::is_base_of<Internal::BaseResourceBinding, BindingType>::value>::type>
		inline void Bind() noexcept
		{
			ConstantBinding<1, 0> obj {};
			Bindings.push_back(&obj);
		}

	public:
		Vector<Internal::BaseResourceBinding*> Bindings;
		InputLayout InputLayout;

		ERHIPrimitiveTopologyType PrimitiveTopologyType;

		ERHIFormat DepthStencilViewFormat{ ERHIFormat::INVALID };
		Vector<ERHIFormat> RenderTargetViewFormats;

		String VSShaderPath;
		String PSShaderPath;
	};
}