#pragma once

#include "core/container/map.h"
#include "core/container/vector.h"
#include "core/string.h"

namespace influx
{
	namespace detail
	{
		class i_cache
		{

		};
	}

	/* 
	* cache containing _type data [Container (vector) & umap(pointers to data)]
	* umap does bookkeeping along a combined hash of 2 keys
	* In loading assets, the first key ideally serves as a filepath 
	* and the second as an optional 'initializer' to allow for multiple variations of loading the same file at path.
	*/
	template <typename _type, typename _key1 = string, typename _key2 = int>
	class cache final : public detail::i_cache
	{
	public:
		using combined_hash = pair_hash<_key1, _key2>;
		using data_map = umap<std::pair<_key1, _key2>, _type*, combined_hash>;
		using data_container = vector<_type>;

		bool contains(const _key1& k1, const _key2& k2 = _key2()) const;

		bool add(const _key1& k1, _type m_data, const _key2& k2 = _key2());
		
		bool remove(const _key1& k1, const _key2& k2 = _key2());

		_type* get(const _key1& k1, const _key2& k2 = _key2()) const;

		_type* get_and_or_add(const _key1& k1, _type m_data, const _key2& k2 = _key2());

	private:
		data_map m_dataMap;
		data_container m_data;
	};

	template<typename _type, typename _key1, typename _key2>
	inline bool cache<_type, _key1, _key2>::contains(const _key1& k1, const _key2& k2) const
	{
		return m_dataMap.contains({ k1, k2 });
	}

	template<typename _type, typename _key1, typename _key2>
	inline bool cache<_type, _key1, _key2>::add(const _key1& k1, _type m_data, const _key2& k2)
	{
		if (!contains(k1, k2))
		{
			// Copy...
			m_data.push_back(m_data);
			m_dataMap[{ k1, k2 }] = &m_data.back();
			return true;
		}

		return false;
	}

	template<typename _type, typename _key1, typename _key2>
	inline bool cache<_type, _key1, _key2>::remove(const _key1& k1, const _key2& k2)
	{
		bool was_removed_from_map = false;
		bool was_removed_from_container = false;

		const bool is_map_empty = m_dataMap.empty();
		const bool is_container_empty = m_data.empty();

		_type* m_data = nullptr; 
		if (!contains(k1, k2) || is_map_empty)
		{
			was_removed_from_map = false;
		}
		else
		{
			m_data = get(k1, k2);
			m_dataMap.erase({ k1, k2 });

			was_removed_from_map = true;
		}
		
		if (is_container_empty)
		{
			auto data_found_in_container = std::find(m_data.begin(), m_data.end(), *m_data);
			if (data_found_in_container != m_data.cend())
			{
				std::iter_swap(data_found_in_container, m_data.end() - 1u);
				m_data.pop_back();

				was_removed_from_container = true;
			}
		}

		return was_removed_from_container && was_removed_from_map;
	}
	
	template<typename _type, typename _key1, typename _key2>
	inline _type* cache<_type, _key1, _key2>::get(const _key1& k1, const _key2& k2) const
	{
		if (!contains(k1, k2))
		{
			return nullptr;
		}

		return m_dataMap.at({ k1, k2 });
	}

	template<typename _type, typename _key1, typename _key2>
	_type* cache<_type, _key1, _key2>::get_and_or_add(const _key1& k1, _type m_data, const _key2& k2)
	{
		if (!contains(k1, k2))
		{
			add(k1, m_data, k2);
		}

		return m_dataMap.at({ k1, k2 });
	}
}


