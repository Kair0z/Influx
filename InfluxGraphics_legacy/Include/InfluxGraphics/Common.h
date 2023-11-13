#pragma once

#ifndef __GR_COMMON_H_
#define __GR_COMMON_H_

#include "Types.h"
#include "RHITypes.h"

namespace influx::Graphics
{
#if 0
	struct NewWrapper {};
	inline void* operator new(uint64, NewWrapper, void* ptr) { return ptr; }
	inline void  operator delete(void*, NewWrapper, void*) {}

#define GR_ALLOC(_SIZE)                     ImGui::MemAlloc(_SIZE)
#define GR_FREE(_PTR)                       ImGui::MemFree(_PTR)
#define GR_PLACEMENT_NEW(_PTR)              new(NewWrapper(), _PTR)
#define GR_NEW(_TYPE)                       new(NewWrapper(), ImGui::MemAlloc(sizeof(_TYPE))) _TYPE

	template<typename T> void IM_DELETE(T* p) { if (p) { p->~T(); ImGui::MemFree(p); } }
#endif
}

#endif