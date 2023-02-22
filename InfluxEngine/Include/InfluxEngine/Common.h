#pragma once

#ifndef __ENGINE_COMMON_H_
#define __ENGINE_COMMON_H_

#include "InfluxCore.h"

#if 1 // PLATFORM_WINDOWS 1
#include "Core/Platform/WindowsPlatform.h"
#else
#endif

#define INFLUX_ENGINE_NUM_TASK_THREADS 4u
#endif