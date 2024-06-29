#pragma once

#ifndef _CORE_CLEANUP_H_
#define _CORE_CLEANUP_H_

namespace influx
{
	template <typename _type>
	void safe_delete(_type*& p)
	{
		if (p != nullptr)
		{
			delete p;
			p = nullptr;
		}
	}
}

#endif