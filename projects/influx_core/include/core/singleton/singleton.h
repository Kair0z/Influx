#pragma once

#ifndef _CORE_SINGLETON_H_
#define _CORE_SINGLETON_H_

namespace influx
{
	template <class _type>
	class singleton
	{
	public:
		inline static _type& get_instance()
		{
			return s_instance;
		}

	private:
		static _type s_instance;
	};

	template <class _type>
	_type singleton<_type>::s_instance = _type{};
}

#endif