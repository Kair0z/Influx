#include "graphics_pch.h"
#include "influx_graphics/device.h"

// null includes
// ...

namespace influx::graphics
{
	// representation of a physical gpu
	class null_physical_device final
		: public base
	{
	public:

	};

	// representation of an interface to create graphics objects
	// based on a given physical gpu
	class null_logical_device final
		: public base
	{
	public:

	};

	class null_device final
		: public device
	{
	public:

	};
}