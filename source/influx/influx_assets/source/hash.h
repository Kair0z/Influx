#pragma once
namespace influx::assets
{
    struct hash_inputs final
    {
        string m_filepath;
        asset_version m_version;
        void* m_data;
        uint64 m_data_size;
    };

    uint64 fnv1a_update(uint64 hash, const void* data, uint64 size)
    {
        const uint8* bytes = static_cast<const uint8*>(data);
        for (uint64 i = 0; i < size; ++i) {
            hash ^= bytes[i];
            hash *= 1099511628211ull; // FNV prime
        }
        return hash;
    }

    uint64 create_hash(const hash_inputs& inputs)
    {
        uint64 hash = 14695981039346656037ull; // FNV offset basis
        hash = fnv1a_update(hash, &inputs.m_version, inputs.m_version.size());
        hash = fnv1a_update(hash, inputs.m_filepath.data(), inputs.m_filepath.size());
        char sep = '\0';
        hash = fnv1a_update(hash, &sep, 1);
        hash = fnv1a_update(hash, inputs.m_data, inputs.m_data_size);
        return hash;
    }
}