#pragma once

#ifndef _PIX_EVENTS_H_
#define _PIX_EVENTS_H_

#ifdef PROFILE
#if _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "WinPixEventRuntime/pix3.h"
#endif
#endif

#endif