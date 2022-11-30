#pragma once

#ifndef _CORE_LOCATOR_H_
#define _CORE_LOCATOR_H_

namespace Influx
{
	template <class _T>
	class Locator final
	{
	public:
		inline static _T* Get()
		{
			return mp_service;
		}

		inline static void Provide(_T* newService)
		{
			mp_service = newService;
		}

		inline static void Unset()
		{
			Provide(nullptr);
		}

	private:
		static _T* mp_service;
	};

	template<class _T>
	_T* Locator<_T>::mp_service = nullptr;
}

#endif
