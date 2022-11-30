#pragma once

#ifndef _CORE_ASSERT_H_
#define _CORE_ASSERT_H_

#include <cassert>

namespace Influx
{
#if _DEBUG
#define FLX_ASSERT(expr) assert(expr)
#else
#define FLX_ASSERT(expr);
#endif
}

#endif