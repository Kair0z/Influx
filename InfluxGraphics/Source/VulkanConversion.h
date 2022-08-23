
#include "VulkanAPI.h"

#define FLX_HANDLE_NOT_IMPLEMENTED assert(false);
namespace Influx::Graphics::Conversion
{
	inline vk::Format ToVulkan(ERHIFormat format)
	{
		switch (format)
		{
		default: FLX_HANDLE_NOT_IMPLEMENTED;
		case ERHIFormat::RGBA_8_Unorm: return vk::Format::eR8G8B8A8Unorm;
		}
	}

	inline vk::SampleCountFlagBits ToVulkan(ERHISampleCount samples)
	{
		switch (samples)
		{
		default: FLX_HANDLE_NOT_IMPLEMENTED;
		case ERHISampleCount::_1: return vk::SampleCountFlagBits::e1;
		}
	}

	inline vk::QueueFlagBits ToVulkan(ERHICommandQueueType type)
	{
		switch (type)
		{
		default: FLX_HANDLE_NOT_IMPLEMENTED;
		case ERHICommandQueueType::Graphics: return vk::QueueFlagBits::eGraphics;
		}
	}

	inline vk::ImageLayout ToVulkan(ERHIResourceState state)
	{
		switch (state)
		{
		default: FLX_HANDLE_NOT_IMPLEMENTED;
		case ERHIResourceState::Present: return vk::ImageLayout::ePresentSrcKHR;
		case ERHIResourceState::Common: return vk::ImageLayout::eGeneral;
		case ERHIResourceState::CopyDest: return vk::ImageLayout::eTransferDstOptimal;
		case ERHIResourceState::CopySource: return vk::ImageLayout::eTransferSrcOptimal;
		case ERHIResourceState::Undefined: return vk::ImageLayout::eUndefined;
		}
	}

	inline vk::AttachmentStoreOp ToVulkan(ERHIRenderPassStoreOp storeOp)
	{
		switch (storeOp)
		{
		default: FLX_HANDLE_NOT_IMPLEMENTED;
		case ERHIRenderPassStoreOp::DontCare:	return vk::AttachmentStoreOp::eDontCare;
		case ERHIRenderPassStoreOp::Store:		return vk::AttachmentStoreOp::eStore;
		}
	}

	inline vk::AttachmentLoadOp ToVulkan(ERHIRenderPassLoadOp loadOp)
	{
		switch (loadOp)
		{
		default: FLX_HANDLE_NOT_IMPLEMENTED;
		case ERHIRenderPassLoadOp::DontCare:	return vk::AttachmentLoadOp::eDontCare;
		case ERHIRenderPassLoadOp::Clear:		return vk::AttachmentLoadOp::eClear;
		case ERHIRenderPassLoadOp::Load:		return vk::AttachmentLoadOp::eLoad;
		}
	}

	inline vk::PipelineBindPoint ToVulkan(ERHIPipelineBindPoint bindPoint)
	{
		switch (bindPoint)
		{
		default: FLX_HANDLE_NOT_IMPLEMENTED; 
		case ERHIPipelineBindPoint::Graphics: return vk::PipelineBindPoint::eGraphics;
		}
	}

	inline vk::PrimitiveTopology ToVulkan(ERHIPrimitiveTopology topology)
	{
		switch (topology)
		{
		default: FLX_HANDLE_NOT_IMPLEMENTED;
		case ERHIPrimitiveTopology::TriangleList: return vk::PrimitiveTopology::eTriangleList;
		}
	}

	inline vk::PolygonMode ToVulkan(ERHIFillMode fillMode)
	{
		switch (fillMode)
		{
		default: FLX_HANDLE_NOT_IMPLEMENTED;
		case ERHIFillMode::Solid: return vk::PolygonMode::eFill;
		}
	}

	inline vk::CullModeFlagBits ToVulkan(ERHICullMode cullMode)
	{
		switch (cullMode)
		{
		default: FLX_HANDLE_NOT_IMPLEMENTED;
		case ERHICullMode::BackFaceCull: return vk::CullModeFlagBits::eBack;
		case ERHICullMode::FrontFaceCull: return vk::CullModeFlagBits::eFront;
		case ERHICullMode::None:			return vk::CullModeFlagBits::eNone;
		}
	}
}