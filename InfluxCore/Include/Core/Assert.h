#pragma once

#ifndef _CORE_ASSERT_H_
#define _CORE_ASSERT_H_

#include <cassert>

namespace influx
{
#if _DEBUG
#define FLX_ASSERT(x) assert(x);
#else
#define FLX_ASSERT(x);
#endif
}

#endif