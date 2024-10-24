#include "influx_rhi.h"

int main()
{
	using namespace influx;

	rhi::device* device = rhi::device::create(rhi::e_api_type::null);

	delete device;
}