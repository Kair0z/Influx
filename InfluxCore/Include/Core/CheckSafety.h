
#ifndef _CORE_CHECKSAFETY_H_
#define _CORE_CHECKSAFETY_H_

#include "Core/Assert.h"

namespace Influx
{
	// Check... 
#if _DEBUG
	#define FLX_CHECK(V) if (V == nullptr) \
							{ \
								assert(false, "Nullptr-access!"); \
							} 

#else
	#define FLX_CHECK(V) 
#endif

}

#endif