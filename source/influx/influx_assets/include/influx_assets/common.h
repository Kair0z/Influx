#pragma once
#if _DLL
#define INFLUX_ASSET_API __declspec(dllexport)
#else
#define INFLUX_ASSET_API __declspec(dllimport)
#endif

#include "core/result.h"
#include "core/basetypes.h"
#include <iostream>

namespace influx::assets
{
	using asset_version = string;
	template <typename _t = char>
	using result = influx::result<_t, const char*>;

	inline static const char* k_flx_extension = ".flx";
	inline static const char* k_flx_meta_extension = ".flx.meta";

	class archiver final
	{
    private:
        bool m_is_read = false;
        vector<byte> m_write_buffer;
        byte const* m_read_buffer = nullptr;
        uint64 m_size = 0;
        uint64 m_offset = 0;

        INFLUX_ASSET_API void write_bytes(const void* data, uint64 bytesize);
        INFLUX_ASSET_API void read_bytes(void* out_data, uint64 bytesize);

    public:
        archiver() = default;
        archiver(const vector<byte>& readbuffer)
            : m_read_buffer{ readbuffer.data() }
            , m_size{ readbuffer.size() } {
            m_is_read = true;
        }

        template<typename _t>
        void serialize(_t& value)
        {
            if (m_is_read) read_bytes(reinterpret_cast<void*>(&value), sizeof(_t));
            else write_bytes(reinterpret_cast<const void*>(&value), sizeof(_t));
        }

        void serialize(string& str)
        {
            if (is_reading())
            {
                uint64 size = 0u;
                serialize(size);
                str.resize(size);
                for (auto& value : str)
                    serialize(value);
            }
            else
            {
                uint64 size = str.size();
                serialize(size);
                for (auto& value : str)
                    serialize(value);
            }
        }

        template <typename _t>
        void serialize(vector<_t>& values)
        {
            if (is_reading())
            {
                uint64 size = 0u;
                serialize(size);
                values.resize(size);
                for (auto& value : values)
                    serialize(value);
            }
            else
            {
                uint64 size = values.size();
                serialize(size);
                for (auto& value : values)
                    serialize(value);
            }
        }
        
        bool is_reading() const { return m_is_read; }

        const vector<byte>& get_writebuffer() const 
        { return m_write_buffer; }
	};
}