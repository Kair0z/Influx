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

namespace influx::rhi
{
	class resource;
	class commandlist;
	class queue;
	class device;
	class command_allocator;
	class buffer;
	class texture2D;
	class texture3D;
	class descheap;

	template <typename _t = char>
	using result = influx::result<_t, const char*>;

	using platform_window_handle = void*;

	// [common types]
	enum class e_commandlist_type : uint8
	{
		graphics
	};
	enum class e_queue_type : uint8
	{
		graphics,
		compute
	};
	enum class e_descriptor : uint8
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
		resource,
		sampler,
		num
	};
	static constexpr uint32 k_num_descriptor_heap_types = static_cast<uint32>(e_descriptor_heap_type::num);
	enum class e_resource_type
	{
		buffer,
		texture2D,
		texture3D
	};
	enum class e_resource_state : uint32
	{
		common			= 0,
		present			= 1 << 0,
		rendertarget	= 1 << 1,
	};
	struct pixel_format final
	{

	};
	struct renderpass_args final
	{

	};
	struct clear final
	{
		math::float4 m_colour;
	};
	using object_native = void*;
	using descriptor = uint64;
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
		commandallocator,
		commandlist,
		fence,
		buffer,
		texture2D,
		texture3D,
		pipeline,
		rootsignature
	};

	// [descs] these are the descriptions of objects by which they are created 
#pragma region create_desc
	struct device_desc final
	{
		object_native m_physdevice = nullptr;
		bool m_debug = false;
	};
	struct queue_desc final
	{
		static queue_desc default_graphics()
		{
			queue_desc desc{};
			desc.m_priority = 0;
			desc.m_type = e_queue_type::graphics;
			return desc;
		}

		static queue_desc default_compute()
		{
			queue_desc desc{};
			desc.m_priority = 0;
			desc.m_type = e_queue_type::compute;
			return desc;
		}
		object_native m_device = nullptr;
		e_queue_type m_type = e_queue_type::graphics;
		int m_priority = 0;
	};
	struct swapchain_desc final
	{
		const device* m_device = nullptr;
		const queue* m_queue = nullptr;

		platform_window_handle m_window;
		pixel_format m_format;
		uint32 m_num_buffers = 3u;
		math::uint2 m_dimensions;

		bool m_own_descriptors = false;
	};
	struct descheap_desc final
	{
		object_native m_device;
		e_descriptor_heap_type m_type;
		uint32 m_num_descriptors = 0u;
	};
	struct commandallocator_desc final
	{
		object_native m_device;
		e_commandlist_type m_type;
	};
	struct commandlist_desc final
	{
		static commandlist_desc default_graphics()
		{
			commandlist_desc desc{};
			desc.m_allocator = nullptr;
			desc.m_own_fence = true;
			desc.m_type = e_commandlist_type::graphics;
			return desc;
		}

		object_native m_device;

		// if m_allocator == nullptr, commandlist will create its own
		object_native m_allocator = nullptr;

		// if true, commandlist will own its own fence
		bool m_own_fence = false;
		
		e_commandlist_type m_type;
	};
	struct fence_desc final
	{
		object_native m_device;
		uint64 m_init_value = 0u;
	};
	struct buffer_desc final
	{
		object_native m_device;
	};
	struct texture2D_desc final
	{
		object_native m_device;
	};
	struct texture3D_desc final
	{
		object_native m_device;
	};
	struct pipeline_desc final
	{

	};
	struct rootsignature_desc final
	{

	};
#pragma endregion
	template <e_object _t>
	using desc_type = std::tuple_element_t<static_cast<uint32>(_t), std::tuple<
		device_desc,
		queue_desc,
		swapchain_desc,
		descheap_desc,
		commandallocator_desc,
		commandlist_desc,
		fence_desc,
		buffer_desc,
		texture2D_desc,
		texture3D_desc,
		pipeline_desc,
		rootsignature_desc>>;

	// [data_types] this is the extra data associated to the objects
#pragma region data
	struct device_data final
	{
		object_native m_physical_device;
		object_native m_factory;

		uint32 m_descriptor_strides[k_num_descriptor_heap_types];
	};
	struct queue_data final
	{

	};
	struct swapchain_data final
	{
		object_native	m_rtv_heap;
		vector<bool>	m_rtv_dirty_list{};
	};
	struct descheap_data final
	{
		vector<bool> m_freelist;
		uint32 m_desc_stride;
	};
	struct commandallocator_data final
	{

	};
	struct commandlist_data final
	{
		object_native m_allocator;
		object_native m_fence;
		uint32 m_fence_complete_value = 0u;
	};
	struct fence_data final
	{

	};
	struct buffer_data final
	{
		uint64 m_bytesize;
		uint64 m_bytestride;
	};
	struct texture2D_data final
	{
		e_resource_state m_previous_state;
		e_resource_state m_current_state;
	};
	struct texture3D_data final
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
		commandallocator_data,
		commandlist_data,
		fence_data,
		buffer_data,
		texture2D_data,
		texture3D_data,
		pipeline_data,
		rootsignature_data>>;

	template <e_object _t>
	class object
	{
	public:
		static constexpr e_object k_type = _t;
		object_native		m_native_object;
		desc_type<_t>		m_desc;
		data_type<_t>		m_data;
	};

	// [class interfaces]
	// use these to make API calls onto the internal objects
	class buffer final : public object<e_object::buffer>
	{
	public:

	};

	class texture2D final : public object<e_object::texture2D>
	{
	public:
		INFLUX_RHI_API result<> transition(commandlist& cmdlist, e_resource_state new_state);
	};

	class texture3D final : public object<e_object::texture3D>
	{
	public:

	};

	class fence final : public object<e_object::fence>
	{
	public:
		// queues a signal command to the queue
		INFLUX_RHI_API result<> queue_signal(uint64 signal_value, const queue& queue);
		INFLUX_RHI_API result<> signal(uint64 value);
		INFLUX_RHI_API result<> wait_for_value(uint64 value);
		INFLUX_RHI_API result<uint64> query_value() const;
	};

	class queue final : public object<e_object::queue>
	{
	public:
		INFLUX_RHI_API result<> submit(const vector<commandlist*>& commandlists) const;
		INFLUX_RHI_API result<> queue_signal(const fence& fence, uint64 signal_value) const;
		INFLUX_RHI_API result<> queue_signal(object_native fence, uint64 signal_value) const;
	};

	class swapchain final : public object<e_object::swapchain>
	{
	public:
		INFLUX_RHI_API result<> present() const;
		INFLUX_RHI_API result<uint32> get_current_backbuffer_index() const;
		INFLUX_RHI_API result<texture2D> get_backbuffer_resource(uint32 index) const;
		INFLUX_RHI_API result<texture2D> get_backbuffer_resource() const;
		INFLUX_RHI_API result<> resize(const math::uint2& new_dim);

		INFLUX_RHI_API bool owns_rtvs() const;
		INFLUX_RHI_API result<descriptor> get_or_create_backbuffer_rtv(const device& device);
	};

	class descheap final : public object<e_object::descriptor_heap>
	{
	public:
		/* returns a descriptor */
		INFLUX_RHI_API result<descriptor> get_cpu_descriptor(uint32 index) const;
		INFLUX_RHI_API result<descriptor> get_gpu_descriptor(uint32 index) const;

		/* allocates a range of descriptors and returns the base index */
		INFLUX_RHI_API result<uint32> allocate(uint32 num_descriptors);

		INFLUX_RHI_API bool is_allocated(uint32 index) const;
		INFLUX_RHI_API result<> free(const vector<descriptor_range>& ranges);
		INFLUX_RHI_API result<> free_all();

		INFLUX_RHI_API bool owns_descriptor(descriptor desc) const;
		INFLUX_RHI_API result<uint32> get_heap_index(descriptor desc) const;
	};

	class commandlist final : public object<e_object::commandlist>
	{
	public:
		INFLUX_RHI_API result<> start(device& device);
		INFLUX_RHI_API result<> start(const command_allocator& allocator);
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
		INFLUX_RHI_API result<> transition_resource(texture2D& resource, e_resource_state new_state);
		INFLUX_RHI_API result<> update_blas();
		INFLUX_RHI_API result<> update_tlas();
		INFLUX_RHI_API result<> copy_resource(const resource& source, resource& desc);
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
	};

	class command_allocator final : public object<e_object::commandallocator>
	{

	};

	class device final : public object<e_object::device>
	{
	public:
		INFLUX_RHI_API static result<device> create(const device_desc& desc);
		INFLUX_RHI_API result<swapchain> create(const swapchain_desc& desc) const;
		INFLUX_RHI_API result<queue> create(const queue_desc& desc) const;
		INFLUX_RHI_API result<command_allocator> create(const commandallocator_desc& desc) const;
		INFLUX_RHI_API result<commandlist> create(const commandlist_desc& desc) const;
		INFLUX_RHI_API result<descheap> create(const descheap_desc& desc) const;
		INFLUX_RHI_API result<> create_rtv(const texture2D& texture, descriptor descriptor) const;
	};
	
	// [creation methods]
	// these are the platform-native object creation methods
	// if you want to work closer to the API, you can just use these to create your objects 
	// then cast them to the type you expect them to be...
	INFLUX_RHI_API result<object_native> create_native(const device_desc& desc);
	INFLUX_RHI_API result<object_native> create_native(const queue_desc& desc);
	INFLUX_RHI_API result<object_native> create_native(const swapchain_desc& desc);
	INFLUX_RHI_API result<object_native> create_native(const descheap_desc& desc);
	INFLUX_RHI_API result<object_native> create_native(const commandallocator_desc& desc);
	INFLUX_RHI_API result<object_native> create_native(const commandlist_desc& desc);
	INFLUX_RHI_API result<object_native> create_native(const fence_desc& desc);
	INFLUX_RHI_API result<object_native> create_native(const buffer_desc& desc);
	INFLUX_RHI_API result<object_native> create_native(const texture2D_desc& desc);
	INFLUX_RHI_API result<object_native> create_native(const texture3D_desc& desc);
	INFLUX_RHI_API result<object_native> create_native(const pipeline_desc& desc);
	INFLUX_RHI_API result<object_native> create_native(const rootsignature_desc& desc);

	// [import]
	INFLUX_RHI_API result<buffer>				import_buffer(object_native native);
	INFLUX_RHI_API result<texture2D>			import_texture2D(object_native native);
	INFLUX_RHI_API result<texture3D>			import_texture3D(object_native native);
	INFLUX_RHI_API result<descheap>				import_descheap(object_native native);
	INFLUX_RHI_API result<commandlist>			import_commandlist(object_native native);
	INFLUX_RHI_API result<command_allocator>	import_allocator(object_native native);

	template <e_object _t>
	result<object<_t>> create(const desc_type<_t>& desc)
	{
		using obj_type = object<_t>;
		using result_type = result<obj_type>;

		obj_type obj{};
		obj.m_native_object = create_native(desc).get();
		obj.m_desc = desc;
		return obj;
	}

	template <typename _t>
	result<_t> create(const desc_type<_t::k_type>& desc)
	{
		using obj_type = _t;
		using result_type = result<obj_type>;

		obj_type obj{};
		obj.m_native_object = create_native(desc).get();
		obj.m_desc = desc;
		return obj;
	}

	template <typename _t> result<_t> import(object_native);
	template<> inline result<buffer> import(object_native native) { return import_buffer(native); }
	template<> inline result<texture2D> import(object_native native) { return import_texture2D(native); }
	template<> inline result<texture3D> import(object_native native) { return import_texture3D(native); }
	template<> inline result<descheap> import(object_native native) { return import_descheap(native); }
	template<> inline result<commandlist> import(object_native native) { return import_commandlist(native); }
	template<> inline result<command_allocator> import(object_native native) { return import_allocator(native); }
}
ENABLE_ENUM_BIT_OPERATORS(influx::rhi::e_resource_state);