#pragma once

namespace Influx
{
	class RHIDevice
	{
	public:
		RHIDevice() = default;
		~RHIDevice() = default;
		RHIDevice(const RHIDevice&) = delete;
		RHIDevice(RHIDevice&&) = delete;
		RHIDevice& operator=(const RHIDevice&) = delete;
		RHIDevice& operator=(RHIDevice&&) = delete;
	};
}


