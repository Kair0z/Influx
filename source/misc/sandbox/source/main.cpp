// influx::core
#include "core/math/random.h"

// influx::rhi
#include "influx_rhi.h"

// influx::platform
#include "influx_platform/window.h"


//Malous stuff
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <thread>

template <typename _t>
void check_result(influx::rhi::result<_t>  result)
{
	if (!result.is_success())
	{
		printf(result.get_unex());
		assert(false);
	}
}

int getRandomNumber() {
	std::srand(std::time(0)); // Seed the random number generator
	return std::rand() % 4;
}
static auto start = std::chrono::high_resolution_clock::now();

void Malou(){
	switch (getRandomNumber())
	{
	case 0:
	{
		printf("Hihi\n");
		break;
	}
	case 1:
	{
		// Store the ending time
		auto end = std::chrono::high_resolution_clock::now();

		// Calculate delta time as float (in seconds)
		std::chrono::duration<float> delta = end - start;
		float deltaTime = delta.count();

		
		printf("This is the amount of time you havent spent with your girlfriend and instead spent compiling! ");
		printf("%.3f\n", deltaTime);
		break;

	}
	case 2:
	{
		printf("I Love You!!! <3 <3 <3 \n");
		break;
	}
	case 3:
	{
		printf("Malou was here!!!!\n");
		break;

	}
	default: {

		break;
	}
	}

}



using namespace influx;

template <typename _t, uint32 _x, uint32 _y, uint64 _ps>
class virtual_texture
{
public:
	static constexpr uint64 k_width = _x;
	static constexpr uint64 k_height = _y;
	static constexpr uint64 k_num_pixels = _x * _y;
	static constexpr uint64 k_bytes_per_page = _ps;
	static constexpr uint64 k_bytes_per_pixel = sizeof(_t);
	static constexpr uint64 k_bytes_total = k_num_pixels * k_bytes_per_pixel;
	static constexpr uint64 k_mbytes_total = k_bytes_total / 1024u / 1024u;
	static constexpr uint64 k_gbytes_total = k_mbytes_total / 1024u;
	static constexpr uint64 k_num_pages = k_bytes_total / k_bytes_per_page;
	static constexpr uint64 k_num_pixels_per_page = k_bytes_per_page / k_bytes_per_pixel;

private:
	// pack our page states into uint32s
	using packed_pages = uint32;
	static constexpr uint64 k_num_packed_page_bits = 32u;
	static constexpr uint32 k_num_packed_pages = k_num_pages / k_num_packed_page_bits;
	packed_pages m_pagetable[k_num_packed_pages] = {};

	struct page final
	{
		_t* m_memory = nullptr;
		uint32 m_page_index = 0u;
	};
	vector<page> m_allocated_pages{};

	void set_page_allocated(uint32 page_index, bool allocated)
	{
		const uint32 pack_index = page_index / k_num_packed_page_bits;
		const uint32 bit_index = page_index % k_num_packed_page_bits;
		m_pagetable[pack_index] |= (1u << bit_index);
	}
	bool is_page_allocated(uint32 page_index) const
	{
		const uint32 pack_index = page_index / k_num_packed_page_bits;
		const uint32 bit_index = page_index % k_num_packed_page_bits;
		return m_pagetable[pack_index] & (1u << bit_index);
	}
	static uint32 coordinate_to_index(const math::uint2& coordinate)
	{
		return (coordinate.y * k_width) + coordinate.x;
	}

	_t* allocate_pages(uint32 page_index, uint32 num_pages)
	{
		// reserve new size
		m_allocated_pages.reserve(m_allocated_pages.size() + num_pages);

		// do the first allocation
		_t* base_ptr = nullptr;
		page new_page{};
		base_ptr = new_page.m_memory = new _t[k_num_pixels_per_page];
		new_page.m_page_index = page_index;
		m_allocated_pages.push_back(new_page);
		set_page_allocated(page_index, true);

		// allocate rest
		for (uint32 i = 1u; i < num_pages; ++i)
		{
			// do the allocation
			new_page.m_memory = new _t[k_num_pixels_per_page];
			new_page.m_page_index = page_index + i;
			m_allocated_pages.push_back(new_page);

			// register allocated
			set_page_allocated(page_index + i, true);
		}

		return base_ptr;
	}

public:
	uint64 get_bytes_allocated() const
	{
		return k_bytes_per_page * m_allocated_pages.size();
	}

	struct sample_info final
	{
		bool m_allocation = true;
	};
	_t& sample(const math::uint2& coordinate, sample_info* out_info = nullptr)
	{
		const uint32 index = coordinate_to_index(coordinate);
		const uint32 page_index = index / k_num_pixels_per_page;
		const uint32 local_index = index % k_num_pixels_per_page;

		_t* page_base = nullptr;
		const bool is_page_cached = is_page_allocated(page_index);
		if (is_page_cached)
		{
			// find the page (todo: maybe a vector isnt the best here)
			for (const auto& page : m_allocated_pages)
			{
				if (page.m_page_index == page_index)
					page_base = page.m_memory;
			}
		}
		else
		{
			page_base = allocate_pages(page_index, 1u);
		}

		// register info
		if (out_info) (*out_info).m_allocation = !is_page_cached;

		return page_base[local_index];
	}
};

int main()
{
	while (true)
	{
		std::this_thread::sleep_for(std::chrono::seconds(1));
		Malou();
	}
}

void _main()
{
	influx::random::seed_random();

	// 16K texture / 64kb page
	// normally, this case is about 256MB of data
	// virtual paged memory allows us to only allocate pages that are actually sampled.
	static constexpr uint32 k_dimensions = 16384;
	using texture_16k = virtual_texture<char, k_dimensions, k_dimensions, 64u * 1024u>;
	texture_16k texture{};
	
	texture_16k::sample_info samp_info{};
	uint32 num_reused_samples = 0u;
	static constexpr uint32 k_num_samples = 64u * 1024u;
	uint32 randomX, randomY;
	for (uint32 i = 0u; i < k_num_samples; ++i)
	{
		// experiment with access patterns
		randomX = random::get_random<uint32>(0, k_dimensions) % k_dimensions;
		randomY = random::get_random<uint32>(0, k_dimensions) % k_dimensions;
		randomX = randomY = i % 4096u;

		// sample & output
		char& sample = texture.sample({ randomX, randomY }, &samp_info);
		std::cout << sample << " ";

		// accumulate info
		if (samp_info.m_allocation == false) 
			num_reused_samples++;
	}

	const float sample_reuse_pc = (float)num_reused_samples / k_num_samples;
	const float memory_footprint_pc = (float)texture.get_bytes_allocated() / texture.k_mbytes_total;

	__debugbreak();
}