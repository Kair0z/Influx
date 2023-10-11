#pragma once

#include "InfluxGraphics/RHITypes.h"

namespace influx::Graphics
{
	namespace Internal
	{
		class IRHIResourceView
		{

		};

		template <ERHIResourceViewType _t>
		class RHIResourceView : public IRHIResourceView
		{
		protected:

		};
	}
}


