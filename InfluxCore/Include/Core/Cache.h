#pragma once

#include "Core/Container/Map.h"
#include "Core/Container/Vector.h"
#include "Core/String.h"

namespace Influx
{
	namespace Internal
	{
		class ICache
		{

		};
	}

	/* 
	* Cache containing _T data [Container (vector) & UMap(pointers to data)]
	* UMap does bookkeeping along a combined hash of 2 keys
	* In loading assets, the first key ideally serves as a filepath 
	* and the second as an optional 'initializer' to allow for multiple variations of loading the same file at path.
	*/
	template <typename _T, typename _K1 = String, typename _K2 = int>
	class Cache final : public Internal::ICache
	{
	public:
		using Key1 = _K1;
		using Key2 = _K2;
		using CombinedHash = PairHash<_K1, _K2>;
		using Data = _T;
		using DataMap = UMap<std::pair<Key1, Key2>, Data*, CombinedHash>;
		using DataContainer = Vector<Data>;

		/* Marking K2 as optional... */
		bool Contains(const Key1& k1, const Key2& k2 = Key2()) const;

		/* Marking K2 as optional... */
		bool Add(const Key1& k1, Data data, const Key2& k2 = Key2());
		
		/* Marking K2 as optional... */
		bool Remove(const Key1& k1, const Key2& k2 = Key2());

		/* Marking K2 as optional... */
		_T* Get(const Key1& k1, const Key2& k2 = Key2()) const;

	private:
		DataMap m_dataMap;
		DataContainer m_data;
	};

	template<typename _T, typename _K1, typename _K2>
	inline bool Cache<_T, _K1, _K2>::Contains(const Key1& k1, const Key2& k2) const
	{
		return m_dataMap.contains({ k1, k2 });
	}

	template<typename _T, typename _K1, typename _K2>
	inline bool Cache<_T, _K1, _K2>::Add(const Key1& k1, Data data, const Key2& k2)
	{
		if (!Contains(k1, k2))
		{
			// Copy...
			m_data.push_back(data);
			m_dataMap.at({ k1, k2 }) = &m_data.back();
			return true;
		}

		return false;
	}

	template<typename _T, typename _K1, typename _K2>
	inline bool Cache<_T, _K1, _K2>::Remove(const Key1& k1, const Key2& k2)
	{
		bool wasSuccesfullyRemovedFromMap = false;
		bool wasSuccesfullyRemovedFromContainer = false;

		const bool isDataMapEmpty = m_dataMap.empty();
		const bool isDataContainerEmpty = m_data.empty();

		Data* data = nullptr; 
		if (!Contains(k1, k2) || isDataMapEmpty)
		{
			wasSuccesfullyRemovedFromMap = false;
		}
		else
		{
			data = Get(k1, k2);
			m_dataMap.erase({ k1, k2 });

			wasSuccesfullyRemovedFromMap = true;
		}
		
		if (isDataContainerEmpty)
		{
			auto dataFoundInContainer = std::find(m_data.begin(), m_data.end(), *data);
			if (dataFoundInContainer != m_data.cend())
			{
				std::iter_swap(dataFoundInContainer, m_data.end() - 1u);
				m_data.pop_back();

				wasSuccesfullyRemovedFromContainer = true;
			}
		}

		return wasSuccesfullyRemovedFromContainer && wasSuccesfullyRemovedFromMap;
	}
	
	template<typename _T, typename _K1, typename _K2>
	inline _T* Cache<_T, _K1, _K2>::Get(const Key1& k1, const Key2& k2) const
	{
		if (!Contains(k1, k2))
		{
			return nullptr;
		}

		return m_dataMap.at({ k1, k2 });
	}
}


