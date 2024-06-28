#pragma once

namespace influx
{
	template <class _t>
	class singleton
	{
	public:
		inline static _t& get_instance()
		{
			return s_instance;
		}

	private:
		static _t s_instance;
	};

	template <class _type>
	_type singleton<_type>::s_instance = _type{};

	// locator is a singleton that doesn't auto create itself.
	// maintains a static pointer to a manually provided object.
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