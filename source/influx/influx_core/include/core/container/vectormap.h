#pragma once

#include "./vector.h"
#include "./map.h"

namespace influx
{
    template <typename _k, typename _t>
    class vectormap final
    {
        using index = unsigned long long;

        umap<_k, index>     m_map;
        vector<_t>          m_vector;

        _t* find(const _k& key) {
            if (!m_map.contains(key))
                return nullptr;

            return &m_vector[m_map[key]];
        }

    public:
        _t const* find(const _k& key) const {
            if (!m_map.contains(key))
                return nullptr;

            return &m_vector[m_map[key]];
        }
        
        bool contains(const _k& key) const {
            return m_map.contains(key);
        }

        void push_back(const _k& key, const _t& value)
        {
            if (contains(key)) return;

            index new_index = m_vector.size();
            m_vector.push_back(value);
            m_map[key] = new_index;
        }

        const vector<_t>& get_vector() const {
            return m_vector;
        }
    };
}