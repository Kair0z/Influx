#pragma once

#ifndef __CORE_SEMAPHORE_H_
#define __CORE_SEMAPHORE_H_

#include "core/basetypes.h"

#include <semaphore>

namespace influx
{
	template <uint8 _n>
	using Semaphore = std::counting_semaphore<_dim>;

	using Semaphore = std::binary_semaphore;
}

#endif