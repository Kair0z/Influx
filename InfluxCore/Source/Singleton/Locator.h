#pragma once

namespace Influx
{
	template <class T>
	class Locator final
	{
	public:
		inline static T* Get()
		{
			return mpService;
		}

		inline static void Provide(T* newService)
		{
			mpService = newService;
		}

		inline static void Unset()
		{
			Provide(nullptr);
		}

	private:
		static T* mpService;
	};

	template<class T>
	T* Locator<T>::mpService = nullptr;
}
