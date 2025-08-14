// influx::rhi
#include "influx_rhi.h"

int main()
{
	using namespace influx;

	// create a device
	auto device = rhi::create<rhi::device>({}).get();

	// create queue
	rhi::queue_desc queue_desc{
		.m_device = device.m_native_object,
		.m_type = rhi::e_queue_type::graphics,
		.m_priority = 0 };
	rhi::queue queue = device.create(queue_desc).get();
	
	// create a commandallocator
	auto commandallocator = device.create(rhi::commandallocator_desc{}).get();
	auto commandlist = device.create(rhi::commandlist_desc{}).get();
	
	commandlist.start(commandallocator);
	commandlist.end();

	queue.submit({ &commandlist });

	rhi::descheap descheap = device.create(rhi::descheap_desc{}).get();
	descheap;
}