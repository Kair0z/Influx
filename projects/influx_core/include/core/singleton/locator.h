#pragma once

#ifndef _CORE_LOCATOR_H_
#define _CORE_LOCATOR_H_

namespace influx
{
	template <class _type>
	class locator final
	{
	public:
		inline static _type* get()
		{
			return mp_service;
		}

		inline static void provide(_type* newService)
		{
			mp_service = newService;
		}

		inline static void unset()
		{
			provide(nullptr);
		}

	private:
		static _type* mp_service;
	};

	template<class _type>
	_type* locator<_type>::mp_service = nullptr;
}

#endif
