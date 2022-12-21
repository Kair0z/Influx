#pragma once

#ifndef _CORE_SINGLETON_H_
#define _CORE_SINGLETON_H_

namespace Influx
{
	template <class _T>
	class Singleton
	{
	public:
		inline static _T& Get()
		{
			return s_instance;
		}

	private:
		static _T s_instance;
	};

	template <class _T>
	_T Singleton<_T>::s_instance = _T{};
}

#endif