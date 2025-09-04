// influx::core
#include "core/math/random.h"
#include "core/time.h"

// influx::rhi
#include "influx_rhi.h"

// influx::platform
#include "influx_platform/window.h"

// STL
#include <cmath>

using namespace influx;

template <typename _t>
void check_result(influx::rhi::result<_t>  result)
{
	if (!result.is_success())
	{
		printf(result.get_unex());
		assert(false);
	}
}

#if MALOU
//Malous stuff
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <thread>
int getRandomNumber() {
	std::srand(std::time(0)); // Seed the random number generator
	return std::rand() % 4;
}
static auto start = std::chrono::high_resolution_clock::now();
void Malou()
{
	while (true)
	{
		std::this_thread::sleep_for(std::chrono::seconds(1));
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
}
#endif

#if VIRTUAL_TEXTURE
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
void virtual_texture_main()
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
#endif

#if VIRTUAL_FUNCTIONS
static float global = 100.0f;
void the_function()
{
	global /= 10.0f;
}
class pa
{
	virtual void buu() = 0;
};
class pb : public pa
{
public:
	virtual void buu() = 0;
};
class pc : public pb
{
public:
	virtual void buu() override
	{
		the_function();
	}
};
class a
{
public:
	virtual void boo()
	{
		the_function();
	}
	virtual void ba()
	{
		the_function();
	}
	virtual void bee()
	{
		the_function();
	}
};
class b : public a
{
public:
	virtual void boo() override
	{
		the_function();
	}
	virtual void ba() override
	{
		the_function();
	}
	virtual void bee() override
	{
		the_function();
	}
};
class c : public b
{
public:
	virtual void boo() override
	{
		the_function();
	}
	virtual void ba() override
	{
		the_function();
	}
	virtual void bee() override
	{
		the_function();
	}
};
void virtual_functions()
{
	auto test = [](uint32 num_iterations)
	{
		// basic
		global = 100.0f;
		auto before = time::get_now();
		for (uint32 i = 0u; i < num_iterations; ++i)
			the_function();
		float ns_basic = time::get_ns_between<float>(time::get_now(), before);
		std::cout << "[basic]: " << std::setprecision(4) << ns_basic << " ns\n";

		// virtual
		global = 100.0f;
		before = time::get_now(); c the_c{};
		for (uint32 i = 0u; i < num_iterations; ++i)
			the_c.bee();
		float ns_virtual = time::get_ns_between<float>(time::get_now(), before);
		std::cout << "[virtual]: " << std::setprecision(4) << ns_virtual << " ns\n";

		// pure virtual
		global = 100.0f;
		before = time::get_now(); pc the_pc{};
		for (uint32 i = 0u; i < num_iterations; ++i)
			the_pc.buu();
		float ns_pvirtual = time::get_ns_between<float>(time::get_now(), before);
		std::cout << "[pure virtual]: " << std::setprecision(4) << ns_pvirtual << " ns\n";

		if (ns_basic > ns_virtual)
		{
			const int pc = std::abs(std::lround((ns_virtual / ns_basic) * 100) - 100);
			std::cout << "the virtual method is " << pc << "% faster!\n";
		}
		else
		{
			const int pc = std::abs(std::lround((ns_virtual / ns_basic) * 100) - 100);
			std::cout << "the virtual method is " << pc << "% slower!\n";
		}
		if (ns_virtual > ns_pvirtual)
		{
			const int pc = std::abs(std::lround((ns_virtual / ns_pvirtual) * 100) - 100);
			std::cout << "the pure virtual method is " << pc << "% faster than virtual!\n";
		}
		else
		{
			const int pc = std::abs(std::lround((ns_virtual / ns_pvirtual) * 100) - 100);
			std::cout << "the pure virtual method is " << pc << "% slower than virtual!\n";
		}
	};

	std::cout << "[virtual functions]\n";

	int num_iterations = 1;
	while (num_iterations > 0)
	{
		std::cout << "enter num calls: ...\n";
		std::cin >> num_iterations;
		if (num_iterations > 0) 
			test(num_iterations);
		std::cout << "\n";
	}
}
#endif

#if RESULTS || 1
static float glob = 100.0f;
static void the_function()
{
	glob /= 3.5f;
#if 0
	if (glob > 30)
		glob = glob + 0.00001f;
#endif
}
static result<> the_function2()
{
	glob /= 3.5f;
#if 0
	if (glob < 30)
		return result<>::make_error("fail");
#endif
	return {};
}
void test_results()
{
	auto test = [](uint32 num_iterations)
	{
		// basic
			glob = 100.0f;
		auto before = time::get_now();
		for (uint32 i = 0u; i < num_iterations; ++i)
			the_function();
		float ns_basic = time::get_ns_between<float>(time::get_now(), before);
		std::cout << "[basic]: " << std::setprecision(4) << ns_basic << " ns\n";

		// virtual
		glob = 100.0f;
		before = time::get_now();
		for (uint32 i = 0u; i < num_iterations; ++i)
			the_function2();
		float ns_result = time::get_ns_between<float>(time::get_now(), before);
		std::cout << "[result]: " << std::setprecision(4) << ns_result << " ns\n";

		if (ns_basic > ns_result)
		{
			const int pc = std::abs(std::lround((ns_result / ns_basic) * 100) - 100);
			std::cout << "the result method is " << pc << "% faster!\n";
		}
		else
		{
			const int pc = std::abs(std::lround((ns_result / ns_basic) * 100) - 100);
			std::cout << "the result method is " << pc << "% slower!\n";
		}
	};

	std::cout << "[results]\n";

	int num_iterations = 1;
	while (num_iterations > 0)
	{
		std::cout << "enter num calls: ...\n";
		std::cin >> num_iterations;
		if (num_iterations > 0)
			test(num_iterations);
		std::cout << "\n";
	}
}
#endif
int main()
{
	test_results();
}