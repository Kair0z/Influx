#pragma once

namespace Influx
{
	template <class T>
	class Singleton final
	{
	public:
		inline static T& Get()
		{
			return sT;
		}

	private:
		static T sT;
	};

	template <class T>
	T Singleton<T>::sT = T{};
}