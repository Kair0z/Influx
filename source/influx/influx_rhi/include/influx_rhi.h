#pragma once

// influx::core
#include "core/result.h"
#include "core/math/vector.h"
#include "core/enum.h"
#include "core/container/vector.h"
#include "core/string.h"

// influx::shader
#include "influx_shader.h"

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

#if INFLUX_RHI_D3D12
struct IDXGIFactory;
struct IDXGIAdapter1;
struct ID3D12Device;
struct ID3D12CommandList;
struct ID3D12CommandQueue;
struct ID3D12CommandAllocator;
struct ID3D12Heap;
struct ID3D12DescriptorHeap;
struct ID3D12Fence;
struct ID3D12Resource;
struct ID3D12Resource;
struct ID3D12PipelineState;
struct ID3D12PipelineState;
#endif

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

#if INFLUX_RHI_D3D12
	using native_instance				= IDXGIFactory*;
	using native_physdevice				= IDXGIAdapter1*;
	using native_device					= ID3D12Device*;
	using native_commandlist			= ID3D12CommandList*;
	using native_queue					= ID3D12CommandQueue*;
	using native_commandpool			= ID3D12CommandAllocator*;
	using native_memoryheap				= ID3D12Heap*;
	using native_descheap				= ID3D12DescriptorHeap*;
	using native_fence					= ID3D12Fence*;
	using native_texture				= ID3D12Resource*;
	using native_buffer					= ID3D12Resource*;
	using native_compute_pipeline		= ID3D12PipelineState*;
	using native_gfx_pipeline			= ID3D12PipelineState*;
	using native_raytracing_pipeline	= object_native;
	using descriptor					= uint64;
#elif INFLUX_RHI_VULKAN

#else
	using native_instance			= object_native;	// IDXGIFactory
	using native_physdevice			= object_native;	// IDXGIAdapter1
	using native_device				= object_native;	// ID3D12Device
	using native_commandlist		= object_native;	// ID3D12CommandList
	using native_queue				= object_native;	// ID3D12CommandQueue
	using native_commandpool		= object_native;	// ID3D12CommandAllocator
	using native_memoryheap			= object_native;	// ID3D12Heap
	using native_descheap			= object_native;	// ID3D12DescriptorHeap
	using native_fence				= object_native;	// ID3D12Fence
	using native_texture			= object_native;	// ID3D12Resource
	using native_buffer				= object_native;	// ID3D12Resource
	using native_compute_pipeline	= object_native;	// ID3D12PipelineState
	using native_gfx_pipeline		= object_native;
	using native_raytracing_pipeline = object_native;
	using descriptor				= uint64;
#endif

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
	enum class e_blend : uint8
	{
		zero = 1,
		one = 2,
		src_color = 3,
		inv_src_color = 4,
		src_alpha = 5,
		inv_src_alpha = 6,
		dest_alpha = 7,
		inv_dest_alpha = 8,
		dest_color = 9,
		inv_dest_color = 10,
		src_alpha_sat = 11,
		blend_factor = 14,
		inv_blend_factor = 15,
		src1_color = 16,
		inv_src1_color = 17,
		src1_alpha = 18,
		inv_src1_alpha = 19,
		alpha_factor = 20,
		inv_alpha_factor = 21,
		count
	};
	enum class e_blendop : uint8
	{
		add = 1,
		subtract = 2,
		rev_subtract = 3,
		min = 4,
		max = 5,
		count
	};
	enum class e_primitive_topology_type : uint8
	{
		triangle = 0,
		point = 1,
		line = 2,
		patch = 3,
		count
	};
	enum class e_primitive_topology : uint8
	{
		trilist,
		linelist,
		count
	};
	enum class e_pipeline_type
	{
		graphics,
		compute,
		raytracing,
		num
	};
	enum class e_hitgroup_type
	{
		triangles,
		count
	};
	enum class e_cull_mode : uint8
	{
		front,
		back,
		nocull,
		count
	};
	enum class e_fill_mode : uint8
	{
		solid,
		wireframe,
		count
	};
	enum class e_comparison_func : uint8
	{
		less,		// <
		lequal,		// <=
		gequal,		// >=
		greater,	// >
		always,
		count
	};
	struct hitgroup final
	{
	public:
		e_hitgroup_type m_type;
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
		rootsignature,
		num
	};

	struct present_args final
	{
		uint32 m_sync_interval;
		uint32 m_flags;
	};
	static constexpr uint32 k_num_descriptor_heap_types = static_cast<uint32>(e_descriptor_heap_type::num);
	static constexpr uint32 k_max_num_rendertargets_per_draw = 8u;

	// =============================================
	// [shaders]
	using shadercode = vector<byte>;

	enum class e_graphics_shader_slots : uint8
	{
		as,	// amp
		ms,	// mesh
		vs,	// vertex
		ps, // pixel
		ds, // domain
		gs,	// geometry
		hs, // hull
		num
	};
	// useful flag-based config presets that outline the valid shader combinations
	enum class e_graphics_shader_pipeline : uint8
	{
		none = 0,
		as = 1 << static_cast<uint32>(e_graphics_shader_slots::as),	// amp
		ms = 1 << static_cast<uint32>(e_graphics_shader_slots::ms),	// mesh
		vs = 1 << static_cast<uint32>(e_graphics_shader_slots::vs),	// vertex
		ps = 1 << static_cast<uint32>(e_graphics_shader_slots::ps), // pixel
		ds = 1 << static_cast<uint32>(e_graphics_shader_slots::ds), // domain
		gs = 1 << static_cast<uint32>(e_graphics_shader_slots::gs),	// geometry
		hs = 1 << static_cast<uint32>(e_graphics_shader_slots::hs), // hull

		// graphics + tessellation
		vs_ps			= vs | ps,
		vs_hs_ds_ps		= vs | ps | hs | ds,
		vs_hs_ds_gs_ps	= vs | hs | ds | gs | ps,
		vs_gs_ps		= vs | gs | ps,

		// graphics + mesh shaders
		ms_ps			= ms | ps,
		as_ms_ps		= as | ms | ps,
		as_ms			= as | ms
	};
	static constexpr bool is_graphics_shader_pipeline_valid(e_graphics_shader_pipeline flags)
	{
		return flags == e_graphics_shader_pipeline::vs_ps
			|| flags == e_graphics_shader_pipeline::vs_hs_ds_ps
			|| flags == e_graphics_shader_pipeline::vs_hs_ds_gs_ps
			|| flags == e_graphics_shader_pipeline::vs_gs_ps
			|| flags == e_graphics_shader_pipeline::ms_ps
			|| flags == e_graphics_shader_pipeline::as_ms_ps
			|| flags == e_graphics_shader_pipeline::as_ms
			|| flags == e_graphics_shader_pipeline::vs
			|| flags == e_graphics_shader_pipeline::ms;
	}
	enum class e_compute_shader_slots : uint8
	{
		cs,
		num
	};
	enum class e_raytracing_shader_slots : uint8
	{
		rgs,
		mss,
		chs,
		ahs,
		ins,
		num
	};
	
	static constexpr uint32 k_num_graphics_shaderslots = static_cast<uint32>(e_graphics_shader_slots::num);
	static constexpr uint32 k_num_compute_shaderslots = static_cast<uint32>(e_compute_shader_slots::num);
	static constexpr uint32 k_num_raytracing_shaderslots = static_cast<uint32>(e_raytracing_shader_slots::num);

	template <e_pipeline_type _t>
	class shader_slots final
	{
	public:
		using enum_type = std::tuple_element_t<static_cast<size_t>(_t), std::tuple<
			e_graphics_shader_slots,
			e_compute_shader_slots,
			e_raytracing_shader_slots>>;

		static constexpr bool is_optional(const enum_type type)
		{
			if constexpr (_t == e_pipeline_type::graphics)
			{
				switch (type)
				{
				case e_graphics_shader_slots::ms: return false;
				case e_graphics_shader_slots::vs: return false;
				case e_graphics_shader_slots::ps: return true;
				case e_graphics_shader_slots::ds: return true;
				case e_graphics_shader_slots::gs: return true;
				case e_graphics_shader_slots::hs: return true;
				default: return false;
				}
			}
			else if constexpr (_t == e_pipeline_type::compute)
			{
				switch (type)
				{
				case e_compute_shader_slots::cs: return false;
				default: return false;
				}
			}
			else if constexpr (_t == e_pipeline_type::raytracing)
			{
				switch (type)
				{
				case e_raytracing_shader_slots::rgs: return false;
				case e_raytracing_shader_slots::mss: return true;
				case e_raytracing_shader_slots::chs: return true;
				case e_raytracing_shader_slots::ahs: return true;
				case e_raytracing_shader_slots::ins: return true;
				default: return false;
				}
			}
		}

		static constexpr bool is_optional(uint8 index)
		{
			return is_optional(static_cast<enum_type>(index));
		}

		inline void set(shader::e_shader_type type, const shadercode& shader_bytecode)
		{
			if constexpr (_t == e_pipeline_type::graphics)
			{
				switch (type)
				{
				case shader::e_shader_type::as: set(e_graphics_shader_slots::as, shader_bytecode); break;
				case shader::e_shader_type::ms: set(e_graphics_shader_slots::ms, shader_bytecode); break;
				case shader::e_shader_type::vs: set(e_graphics_shader_slots::vs, shader_bytecode); break;
				case shader::e_shader_type::ps: set(e_graphics_shader_slots::ps, shader_bytecode); break;
				case shader::e_shader_type::ds: set(e_graphics_shader_slots::ds, shader_bytecode); break;
				case shader::e_shader_type::gs: set(e_graphics_shader_slots::gs, shader_bytecode); break;
				case shader::e_shader_type::hs: set(e_graphics_shader_slots::hs, shader_bytecode); break;
				}
			}
			else if constexpr (_t == e_pipeline_type::compute)
			{
				switch (type)
				{
				case shader::e_shader_type::cs: set(e_compute_shader_slots::cs, shader_bytecode); break;
				}
			}
			else if constexpr (_t == e_pipeline_type::raytracing)
			{
				switch (type)
				{
				case shader::e_shader_type::rgs: set(e_raytracing_shader_slots::rgs, shader_bytecode); break;
				case shader::e_shader_type::mss: set(e_raytracing_shader_slots::mss, shader_bytecode); break;
				case shader::e_shader_type::chs: set(e_raytracing_shader_slots::chs, shader_bytecode); break;
				case shader::e_shader_type::ahs: set(e_raytracing_shader_slots::ahs, shader_bytecode); break;
				case shader::e_shader_type::ins: set(e_raytracing_shader_slots::ins, shader_bytecode); break;
				}
			}
		}

		inline void set(enum_type slot, const shadercode& shader_bytecode)
		{
			m_shaders[static_cast<uint8>(slot)] = shader_bytecode;
		}

		inline const shadercode& get(enum_type slot) const
		{
			return m_shaders[static_cast<uint8>(slot)];
		}

		inline const shadercode& get(uint8 idx) const
		{
			return m_shaders[idx];
		}

		static constexpr uint8 count = static_cast<uint8>(enum_type::num);
		static constexpr uint8 num = count;

	private:
		shadercode m_shaders[count]{};
	};
	
	using graphics_shaderslots		= shader_slots<e_pipeline_type::graphics>;
	using compute_shaderslots		= shader_slots<e_pipeline_type::compute>;
	using raytracing_shaderslots	= shader_slots<e_pipeline_type::raytracing>;

	// =============================================
	// [gfx pipeline]
	struct blend_desc final
	{
		bool m_enabled = false;
		e_blend m_src;
		e_blend m_dest;
		e_blendop m_op;
		e_blend m_srcalpha;
		e_blend m_destalpha;
		e_blendop m_op_alpha;
		uint8 m_write_mask = 0xf; // all

		inline static blend_desc default_write_all()
		{
			blend_desc desc{};
			desc.m_enabled = false;
			desc.m_src;
			desc.m_dest;
			desc.m_op;
			desc.m_srcalpha;
			desc.m_destalpha;
			desc.m_op_alpha;
			desc.m_write_mask = 0xf; // all
			return desc;
		}
	};
	struct sample_desc final
	{

	};
	struct output_merger final
	{
		struct per_rendertarget final
		{
			bool				m_enabled = false;
			pixelformat			m_format = pixelformat::rgba_8_unorm();
			blend_desc			m_blend = blend_desc::default_write_all();
		};
		struct per_depthtarget final
		{
			bool				m_depth_enable	= true;
			bool				m_stencil_enable = false;
			e_comparison_func	m_depth_func = e_comparison_func::less;
			pixelformat			m_format = pixelformat::d32();
		};
		per_rendertarget m_rendertargets[k_max_num_rendertargets_per_draw]{};
		per_depthtarget m_depthtarget{};
	};
	struct rasterizer
	{
		e_cull_mode m_cullmode = e_cull_mode::back;
		e_fill_mode m_fillmode = e_fill_mode::solid;
		bool m_front_ccw = false;
		int m_depth_bias = 0;
		float m_depth_bias_clamp = 0.0f;
		float m_slope_depth_bias = 0.0f;
		bool m_depth_clip_enable = true;
		bool m_multisample = false;
		bool m_antialiased_line = false;
		uint32 m_forced_samplecount = 0u;
		bool m_conservative = false;

		inline static rasterizer default_graphics()
		{
			rasterizer desc{};
			desc.m_cullmode = e_cull_mode::nocull;
			desc.m_fillmode = e_fill_mode::solid;
			desc.m_front_ccw = false;
			desc.m_depth_clip_enable = false;
			desc.m_multisample = false;
			desc.m_antialiased_line = false;
			desc.m_conservative = false;
			desc.m_depth_bias = 0;
			desc.m_depth_bias_clamp = 0.0f;
			desc.m_slope_depth_bias = 0.0f;
			desc.m_forced_samplecount = 0u;
			return desc;
		}
	};

	// TODO PIPELINE ELEMENT FORMATS
	struct graphics_pipeline_desc final
	{
		e_primitive_topology_type	m_primitive_topology_type{};
		output_merger				m_output_merger{};
		rasterizer					m_rasterizer{};
		bool						m_blend_alpha_to_coverage_enabled = false;

		e_graphics_shader_pipeline m_shaderpipeline = e_graphics_shader_pipeline::vs;

		struct input_element final
		{
			string m_semantic_name;
			uint32 m_semantic_idx;
			// e_format m_format;
			uint32 m_input_slot;
			uint32 m_aligned_byteoffset;

			bool m_is_per_instance; // if not, per vertex
			uint32 m_instance_data_steprate;
		};
		vector<input_element> m_input_elements{};
		inline void add_input_element(
			const string& semantic_name,
			uint32 semantic_index,
			// e_format format,
			uint32 input_slot,
			bool is_per_instance,
			uint32 instance_steprate)
		{
			input_element new_element{};
			// new_element.m_format = format;
			new_element.m_input_slot = input_slot;
			new_element.m_instance_data_steprate = instance_steprate;
			new_element.m_is_per_instance = is_per_instance;
			new_element.m_semantic_name = semantic_name;
			new_element.m_semantic_idx = semantic_index;

			// deduce byteoffset
			if (!m_input_elements.empty())
			{
				const input_element& last_element = m_input_elements.back();
#if 0
				new_element.m_aligned_byteoffset =
					last_element.m_aligned_byteoffset + (uint32)deduce_bytesize(last_element.m_format);
#endif
			}

			m_input_elements.push_back(new_element);
		}
	};
	struct raytracing_pipeline_desc final
	{
		uint32 m_max_recursion_depth = 8u;
		vector<string> m_shader_export_names{};
		vector<hitgroup> m_hitgroups{};
	};

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
		optional<native_physdevice> m_physdevice;

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
		native_device			m_device;
	};
	struct pipeline_create_args final
	{
		native_device								m_device;
		e_pipeline_type								m_type{};
		graphics_pipeline_desc						m_graphics{};
		raytracing_pipeline_desc					m_raytracing{};
		shader_slots<e_pipeline_type::graphics>		m_graphics_shaders{};
		shader_slots<e_pipeline_type::compute>		m_compute_shaders{};
		shader_slots<e_pipeline_type::raytracing>	m_raytracing_shaders{};

		inline bool is_valid() const;
	};
	struct rootsignature_create_args final
	{
		native_device m_device;
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

	// =============================================
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

	// =============================================
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
		INFLUX_RHI_API result<descriptor> get_or_create_backbuffer_rtv(device& device);

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

	class pipeline final : public object<e_object::pipeline>
	{

	};
	
	class rootsignature final : public object<e_object::rootsignature>
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
		INFLUX_RHI_API result<> create_rtv(const texture& texture, descriptor descriptor);
		INFLUX_RHI_API result<> create_dsv(const texture& texture, descriptor descriptor);
		INFLUX_RHI_API result<> create_sampview(const sampler& sampler, descriptor descriptor);
		INFLUX_RHI_API result<> create_srv(const texture& texture, descriptor descriptor);
		INFLUX_RHI_API result<> create_uav(const texture& texture, descriptor descriptor);
		INFLUX_RHI_API result<> create_srv(const buffer& buffer, descriptor descriptor);
		INFLUX_RHI_API result<> create_uav(const buffer& buffer, descriptor descriptor);

		inline static result<device>	create(const device_create_args& args = {});
		inline result<swapchain>		create(const swapchain_create_args& args);
		inline result<queue>			create(const queue_create_args& args);
		inline result<commandpool>		create(const commandpool_create_args& args);
		inline result<commandlist>		create(const commandlist_create_args& args);
		inline result<descheap>			create(const descheap_create_args& args);
		inline result<pipeline>			create(const pipeline_create_args& args);
		inline result<rootsignature>	create(const rootsignature_create_args& args);

		inline result<> release()
		{
			return {};
		}

	private:
		inline result<> register_child(const object_native obj)
		{
			using result_type = result<>;
			if (m_data.m_children.contains(obj))
				return result_type::make_error("native object already registered!");

			m_data.m_children.insert(obj);
			return {};
		}
	};

	inline static result<device> create_device(const device_create_args& args = {})
	{
		return device::create(args);
	}

	// =============================================
	/* [import methods]
	* when you import an object (native pointer) the RHI will attempt to build a wrapped type
	* based on the information it can parse from the pointer.
	*/
	INFLUX_RHI_API result<buffer>			import_buffer(native_buffer native);
	INFLUX_RHI_API result<texture>			import_texture(native_texture native);
	INFLUX_RHI_API result<descheap>			import_descheap(native_descheap native);
	INFLUX_RHI_API result<commandlist>		import_commandlist(native_commandlist native);
	INFLUX_RHI_API result<commandpool>		import_commandpool(native_commandpool native);

	// =============================================
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

	// =============================================
	// [inline]
	// device creation methods
	inline result<device> device::create(const device_create_args& args)					
	{ 
		return influx::rhi::create<device>(args);
	}
	inline result<swapchain> device::create(const swapchain_create_args& args)			
	{ 
		using result_type = result<swapchain>;

		auto args_cpy = args; 
		args_cpy.m_device = (native_device)this->m_native_object;
		args_cpy.m_instance = (native_instance)this->m_data.m_instance;
		auto res = influx::rhi::create<swapchain>(args_cpy);
		if (!res)
			return result_type::make_error("failed creating swapchain!");
		
		auto reg = register_child(res.get().m_native_object);
		if (!reg)
			return result_type::make_error("failed registering new object!");
		return res;
	}
	inline result<queue> device::create(const queue_create_args& args)				
	{ 
		using result_type = result<queue>;

		auto args_cpy = args; 
		args_cpy.m_device = (native_device)this->m_native_object; 
		auto res = influx::rhi::create<queue>(args_cpy);
		if (!res)
			return result_type::make_error("failed creating queue!");

		auto reg = register_child(res.get().m_native_object);
		if (!reg)
			return result_type::make_error("failed registering new object!");
		return res;
	}
	inline result<commandpool> device::create(const commandpool_create_args& args)	
	{ 
		using result_type = result<commandpool>;

		auto args_cpy = args; 
		args_cpy.m_device = (native_device)this->m_native_object;
		auto res = influx::rhi::create<commandpool>(args_cpy); 
		if (!res)
			return result_type::make_error("failed creating commandpool!");

		auto reg = register_child(res.get().m_native_object);
		if (!reg)
			return result_type::make_error("failed registering new object!");
		return res;
	}
	inline result<commandlist> device::create(const commandlist_create_args& args)		
	{ 
		using result_type = result<commandlist>;

		auto args_cpy = args; 
		args_cpy.m_device = (native_device)this->m_native_object;
		auto res = influx::rhi::create<commandlist>(args_cpy); 
		if (!res)
			return result_type::make_error("failed creating commandlist!");

		auto reg = register_child(res.get().m_native_object);
		if (!reg)
			return result_type::make_error("failed registering new object!");
		return {};
	}
	inline result<descheap>	device::create(const descheap_create_args& args)			
	{ 
		using result_type = result<descheap>;

		auto args_cpy = args; 
		args_cpy.m_device = (native_device)this->m_native_object;
		auto res = influx::rhi::create<descheap>(args_cpy); 
			return result_type::make_error("failed creating descheap!");

		auto reg = register_child(res.get().m_native_object);
		if (!reg)
			return result_type::make_error("failed registering new object!");
		return res;
	}
	inline result<pipeline> device::create(const pipeline_create_args& args)
	{
		using result_type = result<pipeline>;
		auto args_cpy = args;
		args_cpy.m_device = (native_device)this->m_native_object;
		auto res = influx::rhi::create<pipeline>(args_cpy);
			return result_type::make_error("failed creating pipeline!");

		auto reg = register_child(res.get().m_native_object);
		if (!reg)
			return result_type::make_error("failed registering new object!");
		return res;
	}
	inline result<rootsignature> device::create(const rootsignature_create_args& args)
	{
		using result_type = result<rootsignature>;
		auto args_cpy = args;
		args_cpy.m_device = (native_device)this->m_native_object;
		auto res = influx::rhi::create<rootsignature>(args_cpy);
		if (!res)
			return result_type::make_error("failed creating rootsignature!");

		auto reg = register_child(res.get().m_native_object);
		if (!reg)
			return result_type::make_error("failed registering new object!");
		return res;
	}
}
ENABLE_ENUM_BIT_OPERATORS(influx::rhi::e_resource_state);
ENABLE_ENUM_BIT_OPERATORS(influx::rhi::e_resource_bindflags);
ENABLE_ENUM_BIT_OPERATORS(influx::rhi::e_graphics_shader_pipeline);

namespace influx::rhi
{
	bool pipeline_create_args::is_valid() const
	{
		if (m_type == e_pipeline_type::graphics)
		{
			const e_graphics_shader_pipeline shader_pipeline = m_graphics.m_shaderpipeline;

			// check shader pipeline config...
			const bool shaderpipeline_valid = is_graphics_shader_pipeline_valid(shader_pipeline);
			if (!shaderpipeline_valid) return false;

			// check any shaders that are included & not valid...
			for (uint32 i = 0u; i < k_num_graphics_shaderslots; ++i)
			{
				const bool shader_included = (uint32)shader_pipeline & (1u << i);
				const bool shader_valid = !m_graphics_shaders.get(i).empty();
				if (!shader_valid && shader_included) return false;
			}

			// todo...
			return true;
		}
		if (m_type == e_pipeline_type::compute)
		{
			for (uint32 i = 0u; i < k_num_compute_shaderslots; ++i)
			{
				const bool shader_valid = !m_graphics_shaders.get(i).empty();
				if (!shader_valid) return false;
			}
			return true;
		}
		if (m_type == e_pipeline_type::raytracing)
		{
			for (uint32 i = 0u; i < k_num_raytracing_shaderslots; ++i)
			{
				const bool shader_valid = !m_raytracing_shaders.get(i).empty();
				const bool shader_optional = m_raytracing_shaders.is_optional(i);
				if (!shader_valid && !shader_optional) return false;
			}
			return true;
		}

		// unimplemented type...
		return false;
	}
}