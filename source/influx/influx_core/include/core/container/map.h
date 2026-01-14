#pragma once

#ifndef _CORE_MAP_H_
#define _CORE_MAP_H_

#include <unordered_map>
#include <map>
#include <unordered_set>

namespace influx
{
	template <typename _t>
	using hash = std::hash<_t>;

	template <typename _key1, typename _key2>
	using pair = std::pair<_key1, _key2>;

	template <typename _key1, typename _key2>
	struct pair_hash final
	{
		std::size_t operator()(const pair<_key1, _key2>& pair) const
		{
			return std::hash<_key1>()(pair.first) ^ std::hash<_key2>()(pair.second);
		}
	};

	template <typename _K, typename _t, class _H = std::hash<_K>, class _Eq = std::equal_to<_K>>
	class umap final
	{
		using key_type = _K;
		using value_type = _t;
		using hash_type = _H;
		using equal_to_fn = _Eq;
		using std_umap = std::unordered_map<key_type, value_type, hash_type, equal_to_fn>;

		using iterator = typename std_umap::iterator;
		using const_iterator = typename std_umap::const_iterator;
		using local_iterator = typename std_umap::local_iterator;
		using const_local_iterator = typename std_umap::const_local_iterator;
		std_umap m_std_umap;

	public:
		_t& at(const key_type& key)						{ return m_std_umap.at(key); }
		const _t& at(const key_type& key) const			{ return m_std_umap.at(key); }
		_t& operator[](const key_type& key)				{ return m_std_umap[key]; }
		const _t& operator[](const key_type& key) const { return m_std_umap[key]; }
		void write(const key_type& key, const value_type& value) { at(key) = value; }
		const value_type& read(const key_type& key) const { return at(key); }

		uint64 size() const { return m_std_umap.size(); }

		bool is_keys_equal(const umap& other) const		{ return false; }
		bool is_values_equal(const umap& other) const	{ return false; }
		bool is_equal(const umap& other) const			{ return false; }

		bool contains(const key_type& key) const { return m_std_umap.contains(key); }
		bool empty() const { return m_std_umap.empty(); }

		// std::unordered_map::erase
		void remove(const key_type& key) { m_std_umap.erase(key); }
		void erase(const key_type& key) { m_std_umap.erase(key); }
		void clear() { m_std_umap.clear(); }

		iterator begin() noexcept { return m_std_umap.begin(); }
		const_iterator begin() const noexcept { return m_std_umap.begin(); }
		iterator end() noexcept { return m_std_umap.end(); }
		const_iterator end() const noexcept { return m_std_umap.end(); }
		const_iterator cbegin() const noexcept { return begin(); }
		const_iterator cend() const noexcept { return end(); }
	};

	template <typename K, typename V>
	using map = std::map<K, V>;

	template <typename _k>
	using uset = std::unordered_set<_k>;

	template <typename _k, typename _t>
	bool is_umap_equal(const umap<_k, _t>& a, const umap<_k, _t>& b)
	{
		if (a.size() != b.size()) return false;
		if (a.empty() && b.empty()) return true;

		for (const auto& pair : a)
		{
			if (!b.contains(pair.first)) return false;
			if (b.at(pair.first) != pair.second) return false;
		}

		return true;
	}

#if 0
	template <typename _k, typename _t>
	size_t hash_umap(const const umap<_k, _t>& map)
	{
		size_t counter = 0u;
		size_t result = 0u;
		for (const auto& pair : map)
		{
			const size_t pair_result =
				std::hash<_k>{}(pair.first) ^
				std::hash<_k>{}(pair.second) << 1;

			result ^= pair_result << counter;

			counter++;
		}

		return result;
	}
#endif

	template <typename _t, typename _k, typename _e, _e _max_enm>
	class enummed_map final
	{
		using enum_type = _e;
		using value_type = _t;
		using key_type = _k;
		using hash_type = hash<key_type>;
		using hash_value_type = uint64;

		static constexpr uint32 k_num_enummed = static_cast<uint32>(_max_enm);
		umap<hash_value_type, value_type> m_map;

		static constexpr bool is_enum_hash(const hash_value_type& hash_v)
		{
			return hash_v >= 0u && hash_v < k_num_enummed;
		}
		static constexpr hash_value_type make_enum_hash(const enum_type& enm)
		{
			return static_cast<hash_value_type>(enm);
		}
		static hash_value_type make_key_hash(const key_type& key)
		{
			// make sure key hashes NEVER collide with enum hashes...
			hash_type hasher; return (hasher(key) + k_num_enummed);
		}

	public:
		enummed_map()
		{
			for (uint32 i = 0u; i < k_num_enummed; ++i)
			{
				hash_value_type enm_hash = make_enum_hash((enum_type)i);
				m_map[enm_hash];
			}
		}

		_t& at(const enum_type& enm)
		{
			return m_map[make_enum_hash(enm)];
		}
		const _t& at(const enum_type& enm) const
		{
			return m_map[make_enum_hash(enm)];
		}
		_t& at(const key_type& key)
		{
			return m_map[make_key_hash(key)];
		}
		const _t& at(const key_type& key) const
		{
			return m_map[make_key_hash(key)];
		}
		_t& operator[](const enum_type& enm)
		{
			return at(enm);
		}
		const _t& operator[](const enum_type& enm) const
		{
			return at(enm);
		}
		_t& operator[](const key_type& key)
		{
			return at(key);
		}
		const _t& operator[](const key_type& key) const
		{
			return at(key);
		}
		void write(const enum_type& enm, const value_type& value)
		{
			m_map.write(make_enum_hash(enm), value);
		}
		void write(const key_type& key, const value_type& value)
		{
			m_map.write(make_key_hash(key), value);
		}

		bool contains(const key_type& key) const
		{
			return m_map.contains(make_key_hash(key));
		}
		static constexpr bool contains(const enum_type& enm)
		{
			return true;
		}
	};
}

#endif