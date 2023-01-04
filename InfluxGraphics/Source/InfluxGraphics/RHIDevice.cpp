#include "InfluxGraphics/RHIDevice.h"

namespace Influx::Graphics
{
	RHIDescriptorHeap* RHIDevice::CreateDescriptorHeap(const ERHIDescriptorType type, uint32 numDescriptors, uint8 flags)
	{
		
	}

	RHIDevice::Ptr RHIDevice::Create()
	{
		Ptr newDevice = new RHIDevice();
		newDevice->Initialize();
		return newDevice;
	}

	void RHIDevice::Destroy(RHIDevice::Ptr& device)
	{
		if (device != nullptr)
		{
			delete device;
			device = nullptr;
		}
	}

	RHIDevice::RHIDevice()
	{
		Initialize();
	}

	RHIDevice::~RHIDevice()
	{
		Cleanup();
	}

	void RHIDevice::Initialize()
	{

	}

	void RHIDevice::Cleanup()
	{

	}
}

