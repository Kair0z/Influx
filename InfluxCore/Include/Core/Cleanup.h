#pragma once

#ifndef _CORE_CLEANUP_H_
#define _CORE_CLEANUP_H_

namespace Influx
{
	template <typename _T>
	void SafeDelete(_T*& p)
	{
		delete p;
		p = nullptr;
	}
}

#endif