
#include "VulkanAPI.h"

#define FLX_HANDLE_NOT_IMPLEMENTED throw std::logic_error("Assertion failed!");
namespace Influx::Graphics::Conversion
{
	constexpr vk::Format ToVulkan(ERHIFormat format)
	{
		switch (format)
		{
		default: FLX_HANDLE_NOT_IMPLEMENTED
		}
	}

	constexpr vk::SampleCountFlagBits ToVulkan(ERHISampleCount samples)
	{
		switch (samples)
		{
		default: FLX_HANDLE_NOT_IMPLEMENTED

		}
	}

	constexpr vk::QueueFlagBits ToVulkan(ERHICommandQueueType type)
	{
		switch (type)
		{
		default: FLX_HANDLE_NOT_IMPLEMENTED
		case ERHICommandQueueType::Graphics:
			return vk::QueueFlagBits::eGraphics;
		}
	}

	constexpr vk::ImageLayout ToVulkan(ERHIResourceState state)
	{
		switch (state)
		{
		default: FLX_HANDLE_NOT_IMPLEMENTED
		case ERHIResourceState::Present: return vk::ImageLayout::eSharedPresentKHR;
		case ERHIResourceState::Common: return vk::ImageLayout::eGeneral;
		case ERHIResourceState::CopyDest: return vk::ImageLayout::eTransferDstOptimal;
		case ERHIResourceState::CopySource: return vk::ImageLayout::eTransferSrcOptimal;
		}
	}

	constexpr vk::AttachmentStoreOp ToVulkan(ERHIRenderPassStoreOp storeOp)
	{
		switch (storeOp)
		{
		default: FLX_HANDLE_NOT_IMPLEMENTED
		
		}
	}

	constexpr vk::AttachmentLoadOp ToVulkan(ERHIRenderPassLoadOp loadOp)
	{
		switch (loadOp)
		{
		default: FLX_HANDLE_NOT_IMPLEMENTED
		}
	}

	constexpr vk::PipelineBindPoint ToVulkan(ERHIPipelineBindPoint bindPoint)
	{
		switch (bindPoint)
		{
		default: FLX_HANDLE_NOT_IMPLEMENTED
		}
	}
}