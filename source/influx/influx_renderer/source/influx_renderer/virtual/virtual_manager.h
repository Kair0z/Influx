#pragma once

#include <vector>
#include <unordered_map>
#include <cmath>
#include <bit>

namespace vres
{
	using uint32 = unsigned int;
	using uint64 = unsigned long long;
	using id = uint32;
	using tex_id = id;

	static constexpr uint32 k_invalid_id = (id)-1;

	template <typename _t>
	using vector = std::vector<_t>;
	template <typename _t, typename _k>
	using umap = std::unordered_map<_t, _k>;

	template <typename _t>
	_t ceil(const _t& value) {
		return std::ceil(value);
	}

	constexpr uint32_t deduce_num_mips(uint32 extent) {
		uint32_t levels = 0;
		while (extent > 0) {
			extent >>= 1;
			levels++;
		}
		return levels;
	}

	template <typename _t>
	_t sqrt(_t x) {
		return static_cast<_t>(std::sqrt(static_cast<double>(x)));
	}

	uint32 get_num_active_bits(uint64 value)
	{
		return std::popcount(value);
	}

#define CPP20
#if CPP20
	template <typename _t>
	uint32 get_lowest_set_bit(const _t& value) {
		return std::countr_zero(value);
	}
#endif

	static constexpr uint32 k_expected_max_texture_extent = 4096 * 4u;	// 16K texture
	static constexpr uint32 k_expected_max_num_mips = deduce_num_mips(k_expected_max_texture_extent);
	static constexpr uint32 k_max_pixel_bytesize = 16u;					// 128-bit should be worst case
	static constexpr uint32 k_num_frames_sample_history = 3u;			// 
	static constexpr uint32 k_tile_bytesize = 65536u;					// 64KB

	static constexpr uint32 k_max_num_subresources = 12u;

	static uint64 flatten_3D_coordinate(uint64 x, uint64 y, uint64 z, uint64 width, uint64 height)
	{
		return x + (y * width) + (z * width * height);
	}

	template <typename _t, uint32 _n>
	class ringbuffer final
	{
	private:
		_t m_data[_n];
	public:
		_t& get(uint32 index) {
			return m_data[index % _n];
		}
	};
	
	class bitfield final
	{
		uint64 m_bitfield;
	public:
		// returns true if the value actually ended up changing
		bool set_active(uint32 tile_index, bool active) 
		{
			const bool was_active = is_active(tile_index);
			if (active) m_bitfield |= (1 << tile_index);
			else m_bitfield &= ~(1 << tile_index);
			return is_active(tile_index) != was_active;
		}
		bool is_active(uint32 tile_index) const 
		{
			return (m_bitfield & (1 << tile_index)) != 0u;
		}
		uint64 fill(bool active)
		{
			const uint64 prev_field = m_bitfield;
			m_bitfield = active ? (uint64)-1 : 0u;
			return get_num_active_bits(m_bitfield ^ prev_field);
		}
	};

	class tilefield final
	{
		static constexpr uint32 k_tiles_per_bitfield = 64u;
		vector<bitfield> m_bitfields;

	public:
		tilefield(uint64 num_tiles) 
		{
			set_num_tiles(num_tiles);
		}

		void set_num_tiles(uint64 num_tiles)
		{
			const uint64 num_fields = ceil((float)num_tiles / k_tiles_per_bitfield);
			if (m_bitfields.size() < num_fields)
			{
				m_bitfields.resize(num_fields);
			}
		}

		// returns how many tiles were diffed (set -> unset & unset -> set)
		uint64 set_active(uint64 tile_index, bool active) {
			const uint64 bitfield_index = tile_index / k_tiles_per_bitfield;
			const uint64 local_tile_index = tile_index % k_tiles_per_bitfield;
			const bool changed = m_bitfields[bitfield_index].set_active(local_tile_index, active);
			return changed ? 1u : 0u;
		}

		bool is_active(uint64 tile_index) const {
			const uint64 bitfield_index = tile_index / k_tiles_per_bitfield;
			const uint64 local_tile_index = tile_index % k_tiles_per_bitfield;
			return m_bitfields[bitfield_index].is_active(local_tile_index);
		}

		uint64 set_active(uint64 first_tile, uint64 num_tiles, bool active)
		{
			uint64 sum_changed = 0u;

			const uint64 last_tile = first_tile + num_tiles - 1u;
			const uint64 first_field = first_tile / k_tiles_per_bitfield;
			const uint64 last_field = last_tile / k_tiles_per_bitfield;
			const uint64 local_first_tile = first_tile % k_tiles_per_bitfield;
			if (local_first_tile == 0u) {
				sum_changed += m_bitfields[first_field].fill(active);
			}
			else {
				for (uint64 i = local_first_tile; i < k_tiles_per_bitfield; ++i) {
					sum_changed += m_bitfields[first_field].set_active(i, active) ? 1u : 0u;
				}
			}

			for (uint64 f = first_field + 1u; f < last_field - 1u; ++f) {
				sum_changed += m_bitfields[f].fill(active);
			}

			const uint64 local_last_tile = last_tile % k_tiles_per_bitfield;
			if (local_last_tile >= k_tiles_per_bitfield - 1u) {
				sum_changed += m_bitfields[last_field].fill(active);
			}
			else {
				for (uint64 i = 0u; i <= local_last_tile; ++i) {
					sum_changed += m_bitfields[last_field].set_active(i, active) ? 1u : 0u;
				}
			}
			return sum_changed;
		}

		uint64 clear() 
		{
			uint64 sum_changed = 0u;
			for (bitfield& field : m_bitfields)
			{
				sum_changed += field.fill(false);
			}	
			return sum_changed;
		}
	};

	struct texture_sample final
	{
		// in [0,1] coordinates
		float m_posx;
		float m_posy;
		float m_posz;
		uint32 m_mip_index;
	};

	struct texture_info final
	{
		uint32 m_pixel_bytesize = 1u;
		uint32 m_num_pixels_x;
		uint32 m_num_pixels_y;
		uint32 m_num_pixels_z;
		uint32 m_first_mip = 0u;
		uint32 m_last_mip = 0u;
		
		bool is_valid() const
		{
			return m_num_pixels_x > 0u &&
				m_num_pixels_y > 0u &&
				m_num_pixels_z > 0u &&
				m_pixel_bytesize > 0u &&
				m_first_mip <= m_last_mip &&
				m_last_mip <= k_max_num_subresources;
		}

		uint64 get_num_mips() const {
			return m_last_mip - m_first_mip + 1u;
		}

		bool handle_mip_out_of_bounds(const uint32 in_mip, uint32& out_mip) const {
			if (in_mip < m_first_mip || in_mip > m_last_mip)
				return false;
		}
		uint64 get_num_pixels_per_mip_x(uint32 mip) const {
			if (!handle_mip_out_of_bounds(mip, mip))
				return 0u;

			const uint32 base_num = m_num_pixels_x;
			if (mip == 0u) return base_num;
			else return (base_num / (2u * mip));
		}
		uint64 get_num_pixels_per_mip_y(uint32 mip) const {
			if (!handle_mip_out_of_bounds(mip, mip))
				return 0u;

			const uint32 base_num = m_num_pixels_y;
			if (mip == 0u) return base_num;
			else return (base_num / (2u * mip));
		}
		uint64 get_num_pixels_per_mip_z(uint32 mip) const {
			if (!handle_mip_out_of_bounds(mip, mip))
				return 0u;

			const uint32 base_num = m_num_pixels_z;
			if (mip == 0u) return base_num;
			else return (base_num / (2u * mip));
		}
	};

	struct tiling_info final
	{
		uint32 m_first_subresource = 0u;
		uint32 m_num_subresources = 0u;

		struct per_subres {
			uint32 m_num_tiles_x = 0u;
			uint32 m_num_tiles_y = 0u;
			uint32 m_num_tiles_z = 0u;
			uint64 get_num_tiles() const { return m_num_tiles_x * m_num_tiles_y * m_num_tiles_z; }
		} m_subresources[k_max_num_subresources];

		static tiling_info from_texture(const texture_info& info, const uint64 tile_bytesize) {
			tiling_info result{};
			result.m_num_subresources = info.get_num_mips();
			result.m_first_subresource = info.m_first_mip;
			const uint64 bytesize_per_pixel = info.m_pixel_bytesize;
			for (uint32 i = 0u; i < result.m_num_subresources; ++i)
			{
				const uint64 mip_index = result.m_first_subresource + i;
				const uint64 num_pixels_in_mip_x = info.get_num_pixels_per_mip_x(mip_index);
				const uint64 num_pixels_in_mip_y = info.get_num_pixels_per_mip_y(mip_index);
				const uint64 num_pixels_in_mip_z = info.get_num_pixels_per_mip_z(mip_index);
				result.m_subresources[i].m_num_tiles_x = ceil((num_pixels_in_mip_x * bytesize_per_pixel) / tile_bytesize);
				result.m_subresources[i].m_num_tiles_y = ceil((num_pixels_in_mip_y * bytesize_per_pixel) / tile_bytesize);
				result.m_subresources[i].m_num_tiles_z = ceil((num_pixels_in_mip_z * bytesize_per_pixel) / tile_bytesize);
			}
			return result;
		}

		uint64 get_num_tiles() const
		{
			uint64 sum = 0u;
			for (uint32 i = 0u; i < m_num_subresources; ++i) {
				sum += m_subresources[i].get_num_tiles();
			}
			return sum;
		}

		uint64 get_subresource_tileoffset(const uint64 subres_index) const
		{
			uint64 offset = 0u;
			for (uint64 i = 0u; i <= subres_index; ++i)
			{
				offset += m_subresources[i].get_num_tiles();
			}
			return offset;
		}

		bool is_valid_sample(const texture_sample& sample) const
		{
			return sample.m_posx >= 0.0f && sample.m_posx <= 1.0f &&
				sample.m_posy >= 0.0f && sample.m_posy <= 1.0f &&
				sample.m_posz >= 0.0f && sample.m_posz <= 1.0f &&
				sample.m_mip_index <= m_first_subresource && sample.m_mip_index >= (m_first_subresource + m_num_subresources);
		}

		uint64 get_flat_tile_index(const texture_sample& sample) const
		{
			const uint64 mip = sample.m_mip_index;
			const uint64 offset = get_subresource_tileoffset(mip);
			const uint64 local_mip_index = mip - m_first_subresource;
			const auto& subres = m_subresources[local_mip_index];
			
			const uint64 tilex = floor(sample.m_posx * subres.m_num_tiles_x);
			const uint64 tiley = floor(sample.m_posy * subres.m_num_tiles_y);
			const uint64 tilez = floor(sample.m_posz * subres.m_num_tiles_z);
			return flatten_3D_coordinate(tilex, tiley, tilez, subres.m_num_tiles_x, subres.m_num_tiles_y);
		}

		bool get_subres_tilerange(const uint32 subres_index, uint64& out_tile_offset, uint64& out_tile_num) const
		{
			if (subres_index < m_first_subresource)
				return false;
			if (subres_index >= m_first_subresource + m_num_subresources)
				return false;

			const uint64 local_mip_index = subres_index - m_first_subresource;
			const uint64 offset = get_subresource_tileoffset(local_mip_index);
			const auto& subres = m_subresources[local_mip_index];
			out_tile_offset = offset;
			out_tile_num = subres.get_num_tiles();
		}
	};

	class vres_manager final
	{
		uint64 m_frame = 0u;
		struct texture_entry final
		{
			texture_info m_texture_info;
			tiling_info m_tiling_info;
			tilefield m_tiles;
		};
		vector<texture_entry> m_textures;
		const texture_info& get_texture_info(const tex_id& id) const { return m_textures[id].m_texture_info; }
		const tiling_info& get_tiling_info(const tex_id& id) const { return m_textures[id].m_tiling_info; }
		tilefield& get_tiles(const tex_id& id) { return m_textures[id].m_tiles; }

		uint64 m_num_tiles_to_allocate = 0u;
		uint64 m_num_tiles_to_free = 0u;

	public:
		// call this when you create a virtual texture
		tex_id register_texture(const texture_info& info, const tex_id ovride = k_invalid_id)
		{
			if (!info.is_valid() == false)
			{
				return k_invalid_id;
			}

			if (ovride != k_invalid_id)
			{
				return ovride;
			}
			else
			{	
				// allocate the new entry
				tex_id new_id = m_textures.size();
				texture_entry new_entry{};
				new_entry.m_texture_info = info;
				new_entry.m_tiling_info = tiling_info::from_texture(info);
				new_entry.m_tiles = new_entry.m_tiling_info.get_num_tiles();
				m_textures.push_back(new_entry);
				return new_id;
			}
		}

		// for each sample, set the appropriate tile (this frame) active
		void map_samples(const tex_id& texid, const vector<texture_sample>& samples)
		{
			const texture_info& tex_info = get_texture_info(texid);
			const tiling_info& tile_info = get_tiling_info(texid);
			tilefield& tiles = get_tiles(texid);
			for (const texture_sample& sample : samples) {
				if (tile_info.is_valid_sample(sample))
				{
					const uint64 sampled_tile_index = tile_info.get_flat_tile_index(sample);
					uint64 num_changed = tiles.set_active(sampled_tile_index, true);
					m_num_tiles_to_allocate += num_changed;
				}
			}
		}

		// set all tiles of a mip (this frame) active
		void map_mip(const tex_id& texid, const uint32 mip) {
			
			const texture_info& tex_info = get_texture_info(texid);
			const tiling_info& tile_info = get_tiling_info(texid);
			tilefield& tiles = get_tiles(texid);

			uint64 mip_tile_offset = 0u, mip_num_tiles = 0u;
			bool valid = tile_info.get_subres_tilerange(mip, mip_tile_offset, mip_num_tiles);
			if (valid) {
				m_num_tiles_to_allocate += tiles.set_active(mip_tile_offset, mip_num_tiles, true);
			}
		}

		// update the tile memory heap so that all old allocations (frees) can go,
		// and the new allocations can be assigned to new tiles
		void update_heap()
		{
			// deallocate frees

			// allocate new tiles

		}

		void update_mappings()
		{

		}
	};
}