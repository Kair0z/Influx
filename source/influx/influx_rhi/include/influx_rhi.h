#pragma once

#include "core/result.h"
#include "core/math/vector.h"
#include "core/enum.h"
#include "core/container/vector.h"
#include "core/string.h"

#if _DLL
#define INFLUX_RHI_API __declspec(dllexport)
#else
#define INFLUX_RHI_API __declspec(dllimport)
#endif

#define INFLUX_RHI_VULKAN	0
#define INFLUX_RHI_D3D12	1

#include "influx_rhi/format.h"

// STL
#include <optional>

namespace influx::rhi
{
	class resource;
	class commandlist;
	class queue;
	class device;
	class commandpool;
	class buffer;
	class texture;
	class texture3D;
	class descheap;

	// =============================================
	// native objects
	using object_native			= void*;
	using native_instance		= object_native;	// IDXGIFactory
	using native_physdevice		= object_native;	// ID3D12Adapter
	using native_device			= object_native;	// ID3D12Device
	using native_commandlist	= object_native;	// ID3D12CommandList
	using native_queue			= object_native;	// ID3D12Queue
	using native_commandpool	= object_native;	// ID3D12CommandAllocator
	using native_memoryheap		= object_native;	// ID3D12Heap
	using native_descheap		= object_native;	// ID3D12DescriptorHeap
	using native_fence			= object_native;	// ID3D12Fence
	using native_texture		= object_native;	// ID3D12Resource
	using native_buffer			= object_native;	// ID3D12Resource
	using descriptor			= uint64;

	// =============================================
	// common types
	template <typename _t = char>
	using result = influx::result<_t, const char*>;
	template <typename _t>
	using optional = std::optional<_t>;

	using platform_window_handle = void*;
	enum class e_api : uint8
	{
		d3d12,
		vulkan,
		num
	};
	enum class e_commandlist_type : uint8
	{
		graphics
	};
	enum class e_commandlist_state : uint8
	{
		init,
		recording,
		closed,
		inflight,
		num
	};
	enum class e_queue_type : uint8
	{
		graphics,
		compute
	};
	enum class e_create_descriptor : uint8
	{
		rtv,
		dsv,
		srv,
		uav,
		cbv,
		sampler
	};
	enum class e_command : uint32
	{

	};
	enum class e_descriptor_heap_type : uint8
	{
		rtv,
		dsv,
		rsc, // srv / uav / cbv
		sampler,
		num
	};
	enum class e_resource_type
	{
		buffer,
		texture
	};
	enum class e_texture_type
	{
		texture1D,
		texture2D,
		texture3D,
		cubemap,
		num
	};
	enum class e_resource_state : uint32
	{
		none = 0 << 0,
		common = 1 << 0,
		present = 1 << 1,
		render_target = 1 << 2,
		depth_target = 1 << 3,
		depth_readonly = 1 << 4,
		vs_srv = 1 << 5,
		ps_srv = 1 << 6,
		cs_srv = 1 << 7,
		vs_uav = 1 << 8,
		ps_uav = 1 << 9,
		cs_uav = 1 << 10,
		clear_uav = 1 << 11,
		copy_src = 1 << 12,
		copy_dst = 1 << 13,
		shading_rate = 1 << 14,
		indexbuffer = 1 << 15,
		indirect_args = 1 << 16,
		as_read = 1 << 17,
		as_write = 1 << 18,
		discard = 1 << 19,
		resolve_dst = 1 << 20,
		resolve_src = 1 << 21,

		all_vs = vs_srv | vs_uav,
		all_ps = ps_srv | ps_uav,
		all_cs = cs_srv | cs_uav,
		all_srv = vs_srv | ps_srv | cs_srv,
		all_uav = vs_uav | ps_uav | cs_uav,
		all_depth = depth_target | depth_readonly,
		all_copy = copy_src | copy_dst,
		all_as = as_read | as_write,
		gen_read = copy_src | all_srv,
		gen_write = copy_dst | all_uav,
		all_shading = all_srv | all_uav | shading_rate | as_read
	};
	enum class e_resource_bindflags
	{
		none,
		rtv,
		dsv,
		srv,
		uav
	};
	enum class e_load_op : uint8
	{
		discard,
		preserve,
		clear,
		no_access,
		count
	};
	enum class e_store_op : uint8
	{
		discard,
		preserve,
		resolve,
		no_access,
		count
	};
	struct renderpass_args final
	{

	};
	struct clear final
	{
		static clear colour(const math::float4& colour)
		{
			clear value{};
			value.m_colour = colour;
			return value;
		}
		math::float4 m_colour;
	};
	struct sampler final
	{

	};
	struct descriptor_range final
	{
		descriptor m_base;
		uint32 m_num = 1u;
	};
	enum class e_object : uint8
	{
		device,
		queue,
		swapchain,
		descriptor_heap,
		commandpool,
		commandlist,
		fence,
		buffer,
		texture,
		memoryheap,
		pipeline,
		rootsignature
	};
	struct present_args final
	{
		uint32 m_sync_interval;
		uint32 m_flags;
	};
	static constexpr uint32 k_num_descriptor_heap_types = static_cast<uint32>(e_descriptor_heap_type::num);

	// =============================================
	// [create_args]
#pragma region create_create_args
	struct device_create_args final
	{
		/* unused in D3D12 */
		const char* m_app_name = "";
		const char* m_engine_name = "";
		uint32 m_app_version = 0u;
		uint32 m_engine_version = 0u;
		uint32 m_api_version = 0u;

		/* (optional) specify a physical device for which to create the logic device */
		optional<native_physdevice> m_physdevice = nullptr;

		/* (optional) enable debug systems like validation layers */
		bool m_debug = false;
	};
	struct queue_create_args final
	{
		static queue_create_args default_graphics()
		{
			queue_create_args desc{};
			desc.m_priority = 0;
			desc.m_type = e_queue_type::graphics;
			return desc;
		}
		static queue_create_args default_compute()
		{
			queue_create_args desc{};
			desc.m_priority = 0;
			desc.m_type = e_queue_type::compute;
			return desc;
		}

		native_device m_device = nullptr;
		e_queue_type m_type = e_queue_type::graphics;

		// (optional)
		int m_priority = 0;
	};
	struct swapchain_create_args final
	{
		native_instance m_instance = nullptr;
		native_device	m_device = nullptr;
		native_queue	m_queue = nullptr;

		platform_window_handle	m_window = nullptr;
		pixelformat				m_format = pixelformat::rgba_8_unorm();
		uint32					m_num_buffers = 3u;
		math::uint2				m_dimensions = {};

		// (optional) swapchain will maintain 
		// its own descriptors to the backbuffers
		bool m_own_descriptors = false;
	};
	struct descheap_create_args final
	{
		native_device			m_device;
		e_descriptor_heap_type	m_type;
		uint32					m_num_descriptors = 0u;
		bool					m_shader_visible = false;
	};
	struct commandpool_create_args final
	{
		native_device			m_device;
		e_commandlist_type		m_type;
	};
	struct commandlist_create_args final
	{
		static commandlist_create_args default_graphics()
		{
			commandlist_create_args desc{};
			desc.m_pool = nullptr;
			desc.m_own_fence = true;
			desc.m_type = e_commandlist_type::graphics;
			return desc;
		}

		native_device		m_device;
		e_commandlist_type	m_type;

		// (optional) commandlist will create its own pool
		optional<native_commandpool> m_pool = nullptr;

		// (optional) commandlist will own & carry its own fence
		bool m_own_fence = false;
	};
	struct fence_create_args final
	{
		native_device	m_device;
		uint64			m_init_value = 0u;
	};
	struct buffer_create_args final
	{
		native_device		m_device;
		uint64				m_bytesize;
		uint64				m_bytestride;
		e_resource_bindflags m_bindflags;
		e_resource_state	m_init_state;
		bool				m_allow_uav;

		optional<native_memoryheap> m_heap;
	};
	struct texture_create_args final
	{
		native_device			m_device;
		pixelformat				m_format;
		e_resource_state		m_init_state;

		e_texture_type			m_type;
		uint32					m_arraysize;
		math::uint2				m_dimensions;
		e_resource_bindflags	m_bindflags;
		uint32					m_num_mips;
		uint32					m_sample_count;
		bool					m_allow_uav;

		optional<native_memoryheap> m_heap;
	};
	struct memheap_create_args final
	{

	};
	struct pipeline_create_args final
	{

	};
	struct rootsignature_create_args final
	{

	};
#pragma endregion

	template <e_object _t>
	using create_args = std::tuple_element_t<static_cast<uint32>(_t), std::tuple<
		device_create_args,
		queue_create_args,
		swapchain_create_args,
		descheap_create_args,
		commandpool_create_args,
		commandlist_create_args,
		fence_create_args,
		buffer_create_args,
		texture_create_args,
		memheap_create_args,
		pipeline_create_args,
		rootsignature_create_args>>;

	// [data_types] this is the extra data associated to the objects
#pragma region data
	struct device_data final
	{
		native_instance		m_instance;
		native_physdevice	m_physical_device;
		uint32				m_descriptor_strides[k_num_descriptor_heap_types];
		uset<object_native> m_children{};
	};
	struct queue_data final
	{

	};
	struct swapchain_data final
	{
		native_descheap	m_rtv_heap;
		vector<bool>	m_rtv_dirty_list{};
	};
	struct descheap_data final
	{
		vector<bool> m_freelist;
		uint32 m_descriptor_stride;
	};
	struct commandpool_data final
	{

	};
	struct commandlist_data final
	{
		native_commandpool	m_current_pool;
		native_fence		m_fence;
		uint32				m_fence_complete_value = 0u;
		e_commandlist_state m_state = e_commandlist_state::init;
	};
	struct fence_data final
	{

	};
	struct buffer_data final
	{
		e_resource_state	m_previous_state;
		e_resource_state	m_current_state;
		uint64				m_bytesize;
		uint64				m_bytestride;
	};
	struct texture_data final
	{
		e_resource_state	m_previous_state;
		e_resource_state	m_current_state;
		pixelformat			m_format;
	};
	struct memheap_data final
	{

	};
	struct pipeline_data final
	{

	};
	struct rootsignature_data final
	{

	};
#pragma endregion

	template <e_object _t>
	using data_type = std::tuple_element_t<static_cast<uint32>(_t), std::tuple<
		device_data,
		queue_data,
		swapchain_data,
		descheap_data,
		commandpool_data,
		commandlist_data,
		fence_data,
		buffer_data,
		texture_data,
		memheap_data,
		pipeline_data,
		rootsignature_data>>;

	template <e_object _t>
	class object
	{
	public:
		using data_type = data_type<_t>;
		using create_args = create_args<_t>;

		static constexpr e_object k_type = _t;
		
		inline bool is_valid() const
		{
			return m_native_object != nullptr;
		}

		object() = default;
		object(const object& other) = default;
		object(object&& other) = default;
		object& operator=(const object& other) = default;
		object& operator=(object&& other) = default;
		~object() = default;

		object_native		m_native_object = nullptr;
		create_args			m_create_args = {};
		data_type			m_data = {};
	};

	/* [class interfaces]
	* these are wrapper classes that provide FUNCTIONS on top of the data they store in their base object class.
	* use these to make API calls into the internal objects
	*/

	class resource
	{
	public:
		INFLUX_RHI_API virtual const e_resource_type get_resource_type() const = 0;
		INFLUX_RHI_API virtual const object_native get_native_resource() const = 0;
		INFLUX_RHI_API virtual result<> transition(commandlist& cmdlist, e_resource_state new_state);
		INFLUX_RHI_API virtual e_resource_state get_resource_state() const = 0;
		INFLUX_RHI_API virtual e_resource_state get_previous_resource_state() const = 0;
		INFLUX_RHI_API virtual result<> set_state(e_resource_state new_state) = 0;
		INFLUX_RHI_API virtual bool allows_uav() const = 0;
		INFLUX_RHI_API virtual bool is_valid() const = 0;

		INFLUX_RHI_API virtual uint32 get_arraysize() const { return 0u; };
		INFLUX_RHI_API virtual uint32 get_depth() const { return 0u; };
		INFLUX_RHI_API virtual uint32 get_width() const { return 0u; };
		INFLUX_RHI_API virtual uint32 get_height() const { return 0u; };
		INFLUX_RHI_API virtual uint64 get_bytesize() const { return 0u; };
		INFLUX_RHI_API virtual uint64 get_bytestride() const { return 0u; };
		INFLUX_RHI_API virtual const char* get_name() const { return "";  }

		inline bool is_texture() const
		{
			return get_resource_type() == e_resource_type::texture;
		}
	};

	class buffer final : public object<e_object::buffer>, public resource
	{
	public:
		using data_type = object::data_type;

		INFLUX_RHI_API uint64 get_num_elements() const;
		INFLUX_RHI_API uint64 get_bytesize() const;
		INFLUX_RHI_API uint64 get_bytestride() const;

		// resource interface
		inline virtual const e_resource_type get_resource_type() const override { return e_resource_type::buffer; };
		inline virtual const object_native get_native_resource() const override { return m_native_object; }
		inline virtual e_resource_state get_resource_state() const override { return m_data.m_current_state; }
		inline virtual e_resource_state get_previous_resource_state() const override { return m_data.m_previous_state; }
		inline virtual result<> set_state(e_resource_state new_state) override
		{
			m_data.m_previous_state = m_data.m_current_state;
			m_data.m_current_state = new_state;
			return {};
		}
		inline virtual bool allows_uav() const override { return m_create_args.m_allow_uav; }
		inline virtual bool is_valid() const override { return object::is_valid(); }
	};

	class texture final : public object<e_object::texture>, public resource
	{
	public:
		using data_type = object::data_type;

		INFLUX_RHI_API result<> transition(commandlist& cmdlist, e_resource_state new_state);
		INFLUX_RHI_API result<pixelformat const*> get_current_format() const;

		inline virtual uint32 get_arraysize() const	 override { return 0u; };
		inline virtual uint32 get_depth() const		 override { return 0u; };
		inline virtual uint32 get_width() const		 override { return 0u; };
		inline virtual uint32 get_height() const	 override { return 0u; };
		inline virtual uint64 get_bytesize() const	 override { return 0u; };
		inline virtual uint64 get_bytestride() const override { return 0u; };
		inline virtual const char* get_name() const	override { return ""; }

		// resource interface
		inline virtual const e_resource_type get_resource_type() const override { return e_resource_type::texture; };
		inline virtual const object_native get_native_resource() const override { return m_native_object; }
		inline virtual e_resource_state get_resource_state() const override { return m_data.m_current_state; }
		inline virtual e_resource_state get_previous_resource_state() const override { return m_data.m_previous_state; }
		inline virtual result<> set_state(e_resource_state new_state) override
		{
			m_data.m_previous_state = m_data.m_current_state;
			m_data.m_current_state = new_state;
			return {};
		}
		inline virtual bool allows_uav() const override { return m_create_args.m_allow_uav; }
		inline virtual bool is_valid() const override { return object::is_valid(); }
	};

	class memheap final
	{
	public:

	};

	class fence final : public object<e_object::fence>
	{
	public:
		INFLUX_RHI_API result<>			queue_signal(uint64 signal_value, const queue& queue);
		INFLUX_RHI_API result<>			signal(uint64 value);
		INFLUX_RHI_API result<>			wait_for_value(uint64 value);
		INFLUX_RHI_API result<uint64>	query_value() const;
	};

	class queue final : public object<e_object::queue>
	{
	public:
		inline static queue_create_args default_graphics() { return queue_create_args::default_graphics(); }
		inline static queue_create_args default_compute() { return queue_create_args::default_compute(); }

		INFLUX_RHI_API result<> submit(const vector<commandlist*>& commandlists) const;
		INFLUX_RHI_API result<> queue_signal(const fence& fence, uint64 signal_value) const;
		INFLUX_RHI_API result<> queue_signal(object_native fence, uint64 signal_value) const;
	};

	class swapchain final : public object<e_object::swapchain>
	{
	public:
		INFLUX_RHI_API result<> present(const present_args& args) const;
		INFLUX_RHI_API result<uint32> get_current_backbuffer_index() const;
		INFLUX_RHI_API result<texture> get_backbuffer_resource(uint32 index) const;
		INFLUX_RHI_API result<texture> get_backbuffer_resource() const;
		INFLUX_RHI_API result<> resize(const math::uint2& new_dim);

		INFLUX_RHI_API bool owns_rtvs() const;
		INFLUX_RHI_API result<descriptor> get_or_create_backbuffer_rtv(const device& device);

		INFLUX_RHI_API static bool is_swapchain_format_supported(const pixelformat& format);
		INFLUX_RHI_API static const vector<pixelformat>& get_swapchain_supported_formats();
	
		static pixelformat default_format()
		{
			return pixelformat::rgba_8_unorm();
		}
	};

	class descheap final : public object<e_object::descriptor_heap>
	{
	public:
		using create_args = descheap_create_args;

		/* returns a descriptor */
		INFLUX_RHI_API result<descriptor> get_cpu_descriptor(uint32 index) const;
		INFLUX_RHI_API result<descriptor> get_gpu_descriptor(uint32 index) const;

		/* allocates a range of descriptors and returns the base index */
		INFLUX_RHI_API result<uint32> allocate(uint32 num_create_argsriptors);

		INFLUX_RHI_API bool is_allocated(uint32 index) const;
		INFLUX_RHI_API result<> free(const vector<descriptor_range>& ranges);
		INFLUX_RHI_API result<> free(const descriptor& desc);
		INFLUX_RHI_API result<> free(const uint32 index);
		inline result<> free_all()
		{
			for (uint32 i = 0u; i < m_create_args.m_num_descriptors; ++i)
			{
				auto this_free = free(i);
				if (!this_free) return result<>::make_error("failed freeing all descriptors!");
			}
			return {};
		}

		INFLUX_RHI_API bool owns_descriptor(descriptor desc) const;
		inline bool contains(descriptor desc) const { return owns_descriptor(desc); }
		INFLUX_RHI_API result<uint32> get_heap_index(descriptor desc) const;

		inline e_descriptor_heap_type get_type() const
		{
			return m_create_args.m_type;
		}
	};

	class commandlist final : public object<e_object::commandlist>
	{
	public:
		INFLUX_RHI_API result<> start(device& device);
		INFLUX_RHI_API result<> start(native_commandpool pool);

		INFLUX_RHI_API bool is_recording() const;

		INFLUX_RHI_API result<> renderpass_begin(const renderpass_args& args);
		INFLUX_RHI_API result<> renderpass_end();
		INFLUX_RHI_API result<> draw_instanced();
		INFLUX_RHI_API result<> draw_indexed();
		INFLUX_RHI_API result<> dispatch();
		INFLUX_RHI_API result<> bind_vertexbuffer(const resource& vertexbuffer);
		INFLUX_RHI_API result<> bind_indexbuffer(const resource& indexbuffer);
		INFLUX_RHI_API result<> clear_rtv(descriptor rtv, const clear& clear);
		INFLUX_RHI_API result<> clear_dsv(descriptor dsv);
		INFLUX_RHI_API result<> set_draw_output(descriptor rtv, descriptor dsv);
		INFLUX_RHI_API result<> transition_resource(resource& resource, e_resource_state new_state);
		INFLUX_RHI_API result<> update_blas();
		INFLUX_RHI_API result<> update_tlas();
		INFLUX_RHI_API result<> copy_resource(const resource& source, resource& dest);
		INFLUX_RHI_API result<> bind_descheaps(const vector<const descheap*>& heaps);
		INFLUX_RHI_API result<> bind_rootsignature();
		INFLUX_RHI_API result<> bind_pipeline();
		INFLUX_RHI_API result<> set_viewport();
		INFLUX_RHI_API result<> set_xrect();
		INFLUX_RHI_API result<> set_primitive_topology();
		INFLUX_RHI_API result<> end();

		INFLUX_RHI_API result<> submit(queue& queue);
		INFLUX_RHI_API result<> wait_for_finish() const;
		INFLUX_RHI_API bool has_fence() const;

		inline static commandlist_create_args default_graphics() { return commandlist_create_args::default_graphics(); }
	};

	class commandpool final : public object<e_object::commandpool>
	{

	};
	
	/*
		device can be used to create objects similar to calling the global create-functions
		creating through the device class will automatically override the created objects' m_device reference
		if it has one
	*/
	class device final : public object<e_object::device>
	{
	public:
		INFLUX_RHI_API result<> create_rtv(const texture& texture, descriptor descriptor) const;
		INFLUX_RHI_API result<> create_dsv(const texture& texture, descriptor descriptor) const;
		INFLUX_RHI_API result<> create_sampview(const sampler& sampler, descriptor descriptor) const;
		INFLUX_RHI_API result<> create_srv(const texture& texture, descriptor descriptor) const;
		INFLUX_RHI_API result<> create_uav(const texture& texture, descriptor descriptor) const;
		INFLUX_RHI_API result<> create_srv(const buffer& buffer, descriptor descriptor) const;
		INFLUX_RHI_API result<> create_uav(const buffer& buffer, descriptor descriptor) const;

		inline static result<device>	create(const device_create_args& args = {});
		inline result<swapchain>		create(const swapchain_create_args& args) const;
		inline result<queue>			create(const queue_create_args& args) const;
		inline result<commandpool>		create(const commandpool_create_args& args) const;
		inline result<commandlist>		create(const commandlist_create_args& args) const;
		inline result<descheap>			create(const descheap_create_args& args) const;
	};

	inline static result<device> create_device(const device_create_args& args = {})
	{
		return device::create(args);
	}

	/* [import methods]
	* when you import an object (native pointer) the RHI will attempt to build a wrapped type
	* based on the information it can parse from the pointer.
	*/
	INFLUX_RHI_API result<buffer>			import_buffer(native_buffer native);
	INFLUX_RHI_API result<texture>			import_texture(native_texture native);
	INFLUX_RHI_API result<descheap>			import_descheap(native_descheap native);
	INFLUX_RHI_API result<commandlist>		import_commandlist(native_commandlist native);
	INFLUX_RHI_API result<commandpool>		import_commandpool(native_commandpool native);

	template <typename _t> 
	result<_t> import(object_native);
	template<> inline result<buffer>		import(object_native native) { return import_buffer(native); }
	template<> inline result<texture>		import(object_native native) { return import_texture(native); }
	template<> inline result<descheap>		import(object_native native) { return import_descheap(native); }
	template<> inline result<commandlist>	import(object_native native) { return import_commandlist(native); }
	template<> inline result<commandpool>	import(object_native native) { return import_commandpool(native); }

	/* [creation methods]
	* these are the platform-native object creation methods
	* if you want to work closer to the API, you can just use these to create your objects
	* then cast them to the type you expect them to be...
	* 
	* optionally, you can specify an address 'out_data' to which the create function writes various queried information
	*/ 
	INFLUX_RHI_API result<object_native> create_native(const device_create_args& args, device_data* out_data = nullptr);
	INFLUX_RHI_API result<object_native> create_native(const queue_create_args& args, queue_data* out_data = nullptr);
	INFLUX_RHI_API result<object_native> create_native(const swapchain_create_args& args, swapchain_data* out_data = nullptr);
	INFLUX_RHI_API result<object_native> create_native(const descheap_create_args& args, descheap_data* out_data = nullptr);
	INFLUX_RHI_API result<object_native> create_native(const commandpool_create_args& args, commandpool_data* out_data = nullptr);
	INFLUX_RHI_API result<object_native> create_native(const commandlist_create_args& args, commandlist_data* out_data = nullptr);
	INFLUX_RHI_API result<object_native> create_native(const fence_create_args& args, fence_data* out_data = nullptr);
	INFLUX_RHI_API result<object_native> create_native(const buffer_create_args& args, buffer_data* out_data = nullptr);
	INFLUX_RHI_API result<object_native> create_native(const texture_create_args& args, texture_data* out_data = nullptr);
	INFLUX_RHI_API result<object_native> create_native(const pipeline_create_args& args, pipeline_data* out_data = nullptr);
	INFLUX_RHI_API result<object_native> create_native(const rootsignature_create_args& args, rootsignature_data* out_data = nullptr);

	INFLUX_RHI_API result<> release(object_native native);

	template <typename _t>
	result<_t> create(const create_args<_t::k_type>& args)
	{
		using obj_type = _t;
		using result_type = result<obj_type>;

		obj_type obj{};
		obj.m_create_args = args;

		auto native_create = create_native(args, &obj.m_data);
		if (!native_create) 
			return result_type::make_error(native_create);

		obj.m_native_object = native_create.get();
		return obj;
	}

	// [inline]
	// device creation methods
	inline result<device> device::create(const device_create_args& args)					
	{ 
		return influx::rhi::create<device>(args);
	}
	inline result<swapchain> device::create(const swapchain_create_args& args) const			
	{ 
		auto args_cpy = args; 
		args_cpy.m_device = this->m_native_object;
		args_cpy.m_instance = this->m_data.m_instance;
		return influx::rhi::create<swapchain>(args_cpy);
	}
	inline result<queue> device::create(const queue_create_args& args) const				
	{ 
		auto args_cpy = args; 
		args_cpy.m_device = this->m_native_object; 
		return influx::rhi::create<queue>(args_cpy); 
	}
	inline result<commandpool> device::create(const commandpool_create_args& args) const	
	{ 
		auto args_cpy = args; 
		args_cpy.m_device = this->m_native_object; 
		return influx::rhi::create<commandpool>(args_cpy); 
	}
	inline result<commandlist> device::create(const commandlist_create_args& args) const		
	{ 
		auto args_cpy = args; 
		args_cpy.m_device = this->m_native_object; 
		return influx::rhi::create<commandlist>(args_cpy); 
	}
	inline result<descheap>	device::create(const descheap_create_args& args) const			
	{ 
		auto args_cpy = args; 
		args_cpy.m_device = this->m_native_object; 
		return influx::rhi::create<descheap>(args_cpy); 
	}
}
ENABLE_ENUM_BIT_OPERATORS(influx::rhi::e_resource_state);
ENABLE_ENUM_BIT_OPERATORS(influx::rhi::e_resource_bindflags);