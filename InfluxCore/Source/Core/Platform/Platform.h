#pragma once

#ifndef _CORE_PLATFORM_H_
#define _CORE_PLATFORM_H_

#if PLATFORM_WINDOWS
#include "WindowsPlatform.h"
#else
#include "NullPlatform.h"
#endif

#endif