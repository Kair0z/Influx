#pragma once

#include "InfluxGraphics/RHITypes.h"

namespace Influx::Graphics
{
	namespace Internal
	{
		class IRHIResourceView
		{

		};

		template <ERHIResourceViewType _T>
		class RHIResourceView : public IRHIResourceView
		{
		protected:

		};
	}
}


