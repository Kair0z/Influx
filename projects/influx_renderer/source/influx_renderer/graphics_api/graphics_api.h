#pragma once
#include "core/singleton.h"
#include "core/pointer.h"
#include "core/container/vector.h"

#include "device.h"
#include "commandqueue.h"
#include "commandlist.h"
#include "commandallocator.h"

namespace influx::renderer::api
{
	// interface base class for each object created by our graphics api
	class base
	{
	public:
		inline void* get_native() const
		{
			return mp_native;
		}

		inline void*& get_native()
		{
			return mp_native;
		}

	protected:
		void* mp_native = nullptr;
	};

	// global api manager
	class graphics_api
		: public singleton<graphics_api>
	{
	public:
		// gathers a list of physical devices (gpu's)
		virtual vector<physical_device> get_physical_devices();

		// create a logical interface device based on the given physical device
		virtual logical_device create_logical_device(const physical_device& device) = 0;

	private:
		vector<shared_ptr<base>> mp_children = {};
	};
}