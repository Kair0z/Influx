#pragma once

#ifndef __CORE_SEMAPHORE_H_
#define __CORE_SEMAPHORE_H_

#include "Core/BasicTypes.h"

#include <semaphore>

namespace Influx
{
	template <uint8 _N>
	using Semaphore = std::counting_semaphore<_N>;

	using Semaphore = std::binary_semaphore;
}

#endif