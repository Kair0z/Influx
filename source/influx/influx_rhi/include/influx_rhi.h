#pragma once

// influx::core
#include "core/result.h"
#include "core/math/vector.h"
#include "core/enum.h"
#include "core/container/vector.h"
#include "core/container/queue.h"
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
struct IDXGISwapChain;
struct ID3D12RootSignature;
struct ID3D12PipelineState;
#elif INFLUX_RHI_VULKAN
typedef struct VkInstance_T* VkInstance;
typedef struct VkPhysicalDevice_T* VkPhysicalDevice;
typedef struct VkDevice_T* VkDevice;
typedef struct VkCommandBuffer_T* VkCommandBuffer;
typedef struct VkQueue_T* VkQueue;
typedef struct VkSwapchainKHR_T* VkSwapchainKHR;
typedef struct VkCommandPool_T* VkCommandPool;
struct VkMemoryHeap;
typedef struct VkDescriptorSet_T* VkDescriptorSet;
typedef struct VkFence_T* VkFence;
typedef struct VkSemaphore_T* VkSemaphore;
typedef struct VkImage_T* VkImage;
typedef struct VkBuffer_T* VkBuffer;
typedef struct VkPipeline_T* VkPipeline;
typedef struct VkPipelineLayout_T* VkPipelineLayout;
typedef struct VkDeviceMemory_T* VkDeviceMemory;
typedef struct VkRenderPass_T* VkRenderPass;
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
	class pipeline;
	class rootsignature;

	// =============================================
	// [native objects]
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
	using native_semaphore				= void*;
	using native_texture				= ID3D12Resource*;
	using native_buffer					= ID3D12Resource*;
	using native_compute_pipeline		= ID3D12PipelineState*;
	using native_gfx_pipeline			= ID3D12PipelineState*;
	using native_raytracing_pipeline	= object_native;
	using native_swapchain				= IDXGISwapChain*;
	using native_descriptor				= uint64;
	using native_pipeline				= ID3D12PipelineState*;
	using native_rootsignature			= ID3D12RootSignature*;
	using native_gpu_address			= uint64;
	using native_renderpass				= ID3D12Fence*;
	
#elif INFLUX_RHI_VULKAN
	using native_instance				= VkInstance;
	using native_physdevice				= VkPhysicalDevice;
	using native_device					= VkDevice;
	using native_commandlist			= VkCommandBuffer;
	using native_queue					= VkQueue;
	using native_swapchain				= VkSwapchainKHR;
	using native_commandpool			= VkCommandPool;
	using native_memoryheap				= VkMemoryHeap*;
	using native_descheap				= VkDescriptorSet;
	using native_fence					= VkFence;
	using native_semaphore				= VkSemaphore;
	using native_texture				= VkImage;
	using native_buffer					= VkBuffer;
	using native_pipeline				= VkPipeline;
	using native_rootsignature			= VkPipelineLayout;
	using native_descriptor				= uint64;
	using native_gpu_address			= VkDeviceMemory;
	using native_renderpass				= VkRenderPass;
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
	using native_descriptor			= uint64;
	using native_gpu_address		= object_native;
	using native_renderpass			= object_native;
#endif

	// =============================================
	// [common types]
#pragma region common types
	template <typename _t = char>
	using result = influx::result<_t, const char*>;
	template <typename _t>
	using optional = std::optional<_t>;
	using platform_window_handle = void*;
	using platform_instance_handle = void*;

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
		compute,
		copy,
		present,
		num
	};
	static constexpr uint32 k_num_queue_types = static_cast<uint32>(e_queue_type::num);
	enum class e_queue_flags : uint8
	{
		none		= 0,
		graphics	= 1 << static_cast<uint32>(e_queue_type::graphics),
		compute		= 1 << static_cast<uint32>(e_queue_type::compute),
		copy		= 1 << static_cast<uint32>(e_queue_type::copy),
		all			= graphics | compute | copy
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
	enum class e_fence_type : uint8
	{
		fence,
		semaphore,
		num
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
	enum class e_resource_bindflags : uint16
	{
		none			= 0,
		rtv				= 1 << 0,
		dsv				= 1 << 1,
		srv				= 1 << 2,
		uav				= 1 << 3,
		copysrc			= 1 << 4,
		copydst			= 1 << 5,
		vertexbuffer	= 1 << 6,
		constbuffer		= 1 << 7,
		indexbuffer		= 1 << 8,
		indirectbuffer	= 1 << 9,
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
	enum class e_logic_op : uint8
	{
		clear		= 0,
		set			,
		copy		,
		copy_inv	,
		noop		,
		invert		,
		AND			,
		NAND		,
		OR			,
		NOR			,
		XOR			,
		EQUIV		,
		AND_REV		,
		AND_INV		,
		OR_REV		,
		OR_INV		,
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
	enum class e_border_color : uint8
	{
		black,
		white,
		black_transparent,
		count
	};
	enum class e_texture_wrap_mode : uint8
	{
		wrap,
		mirror,
		clamp,
		border,
		mirror_once,
		count
	};
	enum class e_filter : uint32
	{
		min_mag_mip_point = 0,
		min_mag_point_mip_linear = 0x1,
		min_point_mag_linear_mip_point = 0x4,
		min_point_mag_mip_linear = 0x5,
		min_linear_mag_mip_point = 0x10,
		min_linear_mag_point_mip_linear = 0x11,
		min_mag_linear_mip_point = 0x14,
		min_mag_mip_linear = 0x15,
		anisotropic = 0x55,
		comparison_min_mag_mip_point = 0x80,
		comparison_min_mag_point_mip_linear = 0x81,
		comparison_min_point_mag_linear_mip_point = 0x84,
		comparison_min_point_mag_mip_linear = 0x85,
		comparison_min_linear_mag_mip_point = 0x90,
		comparison_min_linear_mag_point_mip_linear = 0x91,
		comparison_min_mag_linear_mip_point = 0x94,
		comparison_min_mag_mip_linear = 0x95,
		comparison_anisotropic = 0xd5,
		minimum_min_mag_mip_point = 0x100,
		minimum_min_mag_point_mip_linear = 0x101,
		minimum_min_point_mag_linear_mip_point = 0x104,
		minimum_min_point_mag_mip_linear = 0x105,
		minimum_min_linear_mag_mip_point = 0x110,
		minimum_min_linear_mag_point_mip_linear = 0x111,
		minimum_min_mag_linear_mip_point = 0x114,
		minimum_min_mag_mip_linear = 0x115,
		minimum_anisotropic = 0x155,
		maximum_min_mag_mip_point = 0x180,
		maximum_min_mag_point_mip_linear = 0x181,
		maximum_min_point_mag_linear_mip_point = 0x184,
		maximum_min_point_mag_mip_linear = 0x185,
		maximum_min_linear_mag_mip_point = 0x190,
		maximum_min_linear_mag_point_mip_linear = 0x191,
		maximum_min_mag_linear_mip_point = 0x194,
		maximum_min_mag_mip_linear = 0x195,
		maximum_anisotropic = 0x1d5
	};
	enum class e_shader_visibility : uint32
	{
		none = 0,
		vertex = 1 << 0,
		hull = 1 << 1,
		domain = 1 << 2,
		geometry = 1 << 3,
		pixel = 1 << 4,
		compute = 1 << 5,
		amp = 1 << 6,
		mesh = 1 << 7,

		all = vertex | hull | domain | geometry | pixel | compute | amp | mesh
	};
	struct queue_families final
	{
		static constexpr uint32 k_invalid_index = (uint32)-1;
		uint32	m_indices[k_num_queue_types]{};

		queue_families()
		{
			for (uint32 i = 0u; i < k_num_queue_types; ++i)
				m_indices[i] = k_invalid_index;
		}
		inline bool is_set(e_queue_type type) const
		{
			return m_indices[static_cast<uint32>(type)] != k_invalid_index;
		}
		inline void set_index(e_queue_type type, uint32 index)
		{
			m_indices[static_cast<uint32>(type)] = index;
		}
		inline uint32& get_index(e_queue_type type)
		{
			return m_indices[static_cast<uint32>(type)];
		}
		inline uint32 get_index(e_queue_type type) const
		{
			return m_indices[static_cast<uint32>(type)];
		}
	};
	struct hitgroup final
	{
	public:
		e_hitgroup_type m_type;
	};
	struct descriptor final
	{
		native_descriptor m_cpu_address;
		native_descriptor m_gpu_address;
		object_native	m_native_view;
	};
	enum e_renderpass_flags : uint32
	{
		none = 0x0,
		read_only_depth = 0x1,
		read_only_stencil = 0x2,
		allow_uav_write = 0x4,
		suspending = 0x8,
		resuming = 0x10,
	};
	struct color_attachment final
	{
		bool				m_is_enabled;
		e_load_op			m_load;
		e_store_op			m_store;
		math::float4		m_clear;
		pixelformat			m_format;

		struct resolve_params final
		{
			resource* m_source;
			resource* m_dest;
			bool m_keep_source = false;
		} m_resolve{};
	};
	struct depth_attachment final
	{
		bool				m_is_enabled = false;
		pixelformat			m_format = pixelformat::d32();
		e_load_op			m_depth_load;
		e_store_op			m_depth_store;
		float				m_depth_clear = 0.0f;
		e_load_op			m_stencil_load = e_load_op::no_access;
		e_store_op			m_stencil_store = e_store_op::no_access;
		uint8				m_stencil_clear = 0u;
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
	struct present_args final
	{
		uint32 m_sync_interval;
		uint32 m_flags;

		// (vulkan)
		native_device m_device;
		native_queue m_present_queue;
	};
	struct viewport final
	{

	};
	struct xrect final
	{

	};
	struct map_args final
	{
		static map_args full_range()
		{
			map_args args{};
			args.m_offset = 0u;
			args.m_bytesize = (uint32)-1;
			return args;
		}
		uint32 m_offset;
		uint32 m_bytesize;
	};
	enum class e_memoryheap_flags : uint8
	{
		none = 0,
		cpu_visible = 1
	};
	struct memoryheap_desc final
	{
		// D3D12_HEAP_PROPERTIES
		// VK_MEMORY_PROPERTY
		e_memoryheap_flags m_flags = e_memoryheap_flags::none;
		static memoryheap_desc shared()
		{
			memoryheap_desc res;
			res.m_flags = e_memoryheap_flags::cpu_visible;
			return res;
		}
	};
	static constexpr uint32 k_num_descriptor_heap_types = static_cast<uint32>(e_descriptor_heap_type::num);
	static constexpr uint32 k_max_num_rendertargets_per_draw = 8u;

	struct framebuffer_desc final
	{
		math::uint2 m_dimensions;
		color_attachment m_color_attachments[k_max_num_rendertargets_per_draw]{};
		depth_attachment m_depth_attachment{};

		inline bool is_depth_enabled() const
		{
			return m_depth_attachment.m_is_enabled;
		}
		inline uint32 get_num_enabled_colour_targets() const
		{
			uint32 num = 0u;
			for (uint32 i = 0u; i < k_max_num_rendertargets_per_draw; ++i)
				num += (uint32)m_color_attachments[i].m_is_enabled;
			return num;
		}
	};
	struct begin_renderpass_args final
	{
		math::uint2 m_dimensions;
		texture const* m_color_targets[k_max_num_rendertargets_per_draw];
		texture const* m_depth_target;
		
		void set_color(const uint32 index, const texture& texture);
		void set_depth(const texture& texture);
	};
	struct draw_args final
	{
		uint32 m_num_vertices;
		uint32 m_num_instances;
		uint32 m_start_vertex;
		uint32 m_start_instance;
	};
	struct draw_indexed_args final
	{
		uint32 m_num_indices;
		uint32 m_num_instances;
		uint32 m_start_index;
		uint32 m_start_vertex;
		uint32 m_start_instance;
	};
#pragma endregion

	enum class e_object : uint8
	{
		device,
		queue,
		swapchain,
		descriptor_heap,
		commandpool,
		commandlist,
		fence,
		semaphore,
		buffer,
		texture,
		memoryheap,
		pipeline,
		rootsignature,
		renderpass,
		num
	};

	// =============================================
	// [shaders]
#pragma region shaders
	using shadercode = vector<byte>;
	struct shaderinfo final
	{
		string m_name;
	};

	enum class e_graphics_shader_slots : uint8
	{
		vs,	// vertex
		ps, // pixel
		ds, // domain
		gs,	// geometry
		hs, // hull
		as,	// amp
		ms,	// mesh
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
				case e_graphics_shader_slots::ms: return true;
				case e_graphics_shader_slots::vs: return false;
				case e_graphics_shader_slots::ps: return true;
				case e_graphics_shader_slots::ds: return true;
				case e_graphics_shader_slots::gs: return true;
				case e_graphics_shader_slots::hs: return true;
				case e_graphics_shader_slots::as: return true;
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

		static constexpr enum_type get_type_at_index(uint8 index)
		{
			return static_cast<enum_type>(index);
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

		inline void set(enum_type slot, const shaderinfo& shader_info)
		{
			m_shaderinfos[static_cast<uint8>(slot)] = shader_info;
		}

		inline const shadercode& get(enum_type slot) const
		{
			return m_shaders[static_cast<uint8>(slot)];
		}

		inline const shadercode& get(uint8 idx) const
		{
			return m_shaders[idx];
		}
		inline const shaderinfo& get_info(uint8 index) const
		{
			return m_shaderinfos[index];
		}

		inline bool is_set(uint8 index) const
		{
			return m_shaders[index].size() > 0u;
		}

		static constexpr uint8 count = static_cast<uint8>(enum_type::num);
		static constexpr uint8 num = count;

	private:
		shadercode m_shaders[count]{};
		shaderinfo m_shaderinfos[count]{};
	};
	
	using graphics_shaderslots		= shader_slots<e_pipeline_type::graphics>;
	using compute_shaderslots		= shader_slots<e_pipeline_type::compute>;
	using raytracing_shaderslots	= shader_slots<e_pipeline_type::raytracing>;
#pragma endregion

	// =============================================
	// [gfx pipeline]
#pragma region pipelines
	struct blend_desc final
	{
		bool m_enabled = false;
		bool m_logic_enabled = false;
		e_blend m_src;
		e_blend m_dest;
		e_blendop m_op;
		e_blend m_srcalpha;
		e_blend m_destalpha;
		e_blendop m_op_alpha;
		e_logic_op m_logic_op;
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
		uint32 m_num_samples = 1u;
		uint32 m_quality = 0u;
		uint32 m_sample_mask = 0xffffffff;
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

		uint32 get_num_enabled_rendertargets() const
		{
			uint32 num = 0u;
			for (uint32 i = 0u; i < k_max_num_rendertargets_per_draw; ++i)
				if (m_rendertargets[i].m_enabled) ++num;
			return num;
		}

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
		sample_desc					m_sample_desc{};
		viewport					m_default_viewport;
		xrect						m_default_xrect;
		e_graphics_shader_pipeline m_shaderpipeline = e_graphics_shader_pipeline::vs;

		bool						m_blend_alpha_to_coverage_enabled = false;
		bool						m_blend_independent;

		struct input_element final
		{
			string			m_semantic_name;
			uint32			m_semantic_idx;
			bufferformat	m_format;
			uint32			m_input_slot;
			uint32			m_aligned_byteoffset;

			bool m_is_per_instance; // if not, per vertex
			uint32 m_instance_data_steprate;
		};
		vector<input_element> m_input_elements{};

		inline void add_input_element(
			const string& semantic_name,
			uint32 semantic_index,
			bufferformat format,
			uint32 input_slot,
			bool is_per_instance,
			uint32 instance_steprate)
		{
			input_element new_element{};
			new_element.m_format = format;
			new_element.m_input_slot = input_slot;
			new_element.m_instance_data_steprate = instance_steprate;
			new_element.m_is_per_instance = is_per_instance;
			new_element.m_semantic_name = semantic_name;
			new_element.m_semantic_idx = semantic_index;

			// deduce byteoffset
			if (!m_input_elements.empty())
			{
				const input_element& last_element = m_input_elements.back();
				new_element.m_aligned_byteoffset =
					last_element.m_aligned_byteoffset + (uint32)last_element.m_format.get_bytesize();
			}

			m_input_elements.push_back(new_element);
		}

		inline void reflect_input_elements(const shader::reflection& reflection)
		{
			uint32 index = 0u;
			for (const auto& element : reflection.m_input_params)
			{
				bufferformat format{};
				switch (element.m_element_type)
				{
				case shader::reflection::input_param::e_component_type::f32: format = bufferformat::make_uint32(element.m_num_floats);
				case shader::reflection::input_param::e_component_type::u32: format = bufferformat::make_f32(element.m_num_floats);
				}

				add_input_element(
					element.m_semantic_name,
					element.m_semantic_index,
					format,
					index,
					false,
					0u);
			}
		}
	};
	struct raytracing_pipeline_desc final
	{
		uint32 m_max_recursion_depth = 8u;
		vector<string> m_shader_export_names{};
		vector<hitgroup> m_hitgroups{};
	};
#pragma endregion

	// =============================================
	// [create_args]
#pragma region create_create_args
	struct device_create_args final
	{
		/* (vulkan) */
		const char* m_app_name = "";
		const char* m_engine_name = "";
		uint32 m_app_version = 0u;
		uint32 m_engine_version = 0u;
		uint32 m_api_version = 0u;

		/* (optional) specify a physical device for which to create the logic device */
		optional<native_physdevice> m_physdevice;

		/* (optional) enable debug systems like validation layers */
		bool m_debug = false;

		static device_create_args make(const string& name, bool debug)
		{
			return device_create_args{
				.m_app_name = name.c_str(),
				.m_engine_name = name.c_str(),
				.m_app_version = {},
				.m_engine_version = {},
				.m_api_version = {},
				.m_physdevice = {},
				.m_debug = debug
			};
		}
	};
	struct queue_create_args final
	{
		native_device m_device = nullptr;
		e_queue_type m_type = e_queue_type::graphics;
		queue_families m_queue_families{};

		// (optional)
		int m_priority = 0;

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
	};
	struct swapchain_create_args final
	{
		native_instance m_instance = nullptr;
		native_device	m_device = nullptr;
		native_queue	m_queue = nullptr;

		// (vulkan)
		queue_families m_queue_families{};

		platform_instance_handle m_platform_instance = nullptr;
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

		// (vulkan)
		queue_families			m_queue_families{};
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
		
		// (vulkan)
		queue_families		m_queue_families{};

		// (optional) commandlist will create its own pool
		optional<native_commandpool> m_pool = nullptr;

		// (optional) commandlist will own & carry its own fence
		bool m_own_fence = false;
	};
	struct fence_create_args final
	{
		static fence_create_args semaphore()
		{
			fence_create_args args{};
			args.m_type = e_fence_type::semaphore;
			return args;
		}
		static fence_create_args fence()
		{
			fence_create_args args{};
			args.m_type = e_fence_type::fence;
			return args;
		}

		native_device	m_device;
		uint64			m_init_value = 0u;
		e_fence_type	m_type = e_fence_type::fence;
	};
	struct semaphore_create_args final
	{
		native_device m_device;
	};
	struct buffer_create_args final
	{
		native_physdevice		m_physdevice;
		native_device			m_device;
		uint64					m_bytesize;
		uint64					m_bytestride;
		e_resource_bindflags	m_bindflags;
		e_resource_state		m_init_state;
		memoryheap_desc			m_memoryheap;
	};
	struct texture_create_args final
	{
		native_physdevice			m_physdevice;
		native_device				m_device;
		pixelformat					m_format = pixelformat::rgba_8_unorm();
		math::uint3					m_dimensions = math::uint3::make_one();
		uint32						m_arraysize = 1u;
		uint32						m_num_mips = 1u;
		uint32						m_sample_count = 1u;
		e_resource_bindflags		m_bindflags;
		e_resource_state			m_init_state;
		e_texture_type				m_type;
		memoryheap_desc				m_memoryheap;
		
		optional<const char*>		m_name;
		bool						m_create_view = true;

		static texture_create_args tex1D(const uint32 num_pixels)
		{
			texture_create_args args{};
			args.m_type = e_texture_type::texture1D;
			args.m_dimensions.x = num_pixels;
			return args;
		}
		static texture_create_args tex2D(const math::uint2& dimensions)
		{
			texture_create_args args{};
			args.m_type = e_texture_type::texture2D;
			args.m_dimensions.x = dimensions.x;
			args.m_dimensions.y = dimensions.y;
			return args;
		}
		static texture_create_args tex3D(const math::uint3& dimensions)
		{
			texture_create_args args{};
			args.m_type = e_texture_type::texture3D;
			args.m_dimensions = dimensions;
			return args;
		}
		static texture_create_args cubemap(const math::uint2& dimensions)
		{
			texture_create_args args{};
			args.m_type = e_texture_type::cubemap;
			args.m_dimensions.x = dimensions.x;
			args.m_dimensions.y = dimensions.y;
			args.m_arraysize = 6u;
			return args;
		}
		static texture_create_args tex2D_depth(const math::uint2& dimensions)
		{
			texture_create_args args = tex2D(dimensions);
			args.mod_format(pixelformat::d32())
				.mod_bindflags(e_resource_bindflags::dsv);
			return args;
		}
		static texture_create_args tex2D_depthstencil(const math::uint2& dimensions)
		{
			texture_create_args args = tex2D(dimensions);
			args.mod_format(pixelformat::d24s8())
				.mod_bindflags(e_resource_bindflags::dsv);
			return args;
		}

		texture_create_args& mod_device(native_device dev)
		{ m_device = dev; return *this; }
		texture_create_args& mod_format(const pixelformat& format)
		{ m_format = format; return *this; }
		texture_create_args& mod_dimensions(const math::uint3& dimensions)
		{ m_dimensions = dimensions; return *this; }
		texture_create_args& mod_arraysize(const uint32 arraysize)
		{ m_arraysize = arraysize; return *this; }
		texture_create_args& mod_num_mips(const uint32 num_mips)
		{ m_num_mips = num_mips; return *this; }
		texture_create_args& mod_samplecount(const uint32 num_samples)
		{ m_sample_count = num_samples; return *this; }
		texture_create_args& mod_bindflags(const e_resource_bindflags flags)
		{ m_bindflags = flags; return *this; }
		texture_create_args& mod_initstate(const e_resource_state state)
		{ m_init_state = state; return *this; }
		texture_create_args& mod_type(const e_texture_type type)
		{ m_type = type; return *this; }
	};
	struct memheap_create_args final
	{
		native_device			m_device;
	};
	struct pipeline_create_args final
	{
		native_rootsignature						m_rootsignature;
		native_device								m_device;
		native_renderpass							m_renderpass;
		e_pipeline_type								m_type{};
		graphics_pipeline_desc						m_graphics{};
		raytracing_pipeline_desc					m_raytracing{};

		graphics_shaderslots m_graphics_shaders;
		compute_shaderslots m_compute_shaders;
		raytracing_shaderslots m_raytracing_shaders;

		inline bool is_valid() const;
	};

	enum class e_rootparam_type : uint32
	{
		descriptor_table,
		constants,
		cbv,
		srv,
		uav,
		count
	};
	struct root_param_common final
	{
		e_shader_visibility m_visibility = e_shader_visibility::all;
		uint32 m_shader_register = 0u;
		uint32 m_register_space = 0u;
		string m_name;
	};
	struct root_param_constants final
	{
		root_param_constants() = default;
		root_param_constants(
			uint32 num_dwords,
			uint32 shader_register,
			uint32 register_space = 0,
			e_shader_visibility visibility = e_shader_visibility::all)
		{
			m_num_dwords = num_dwords;
			m_common.m_register_space = register_space;
			m_common.m_shader_register = shader_register;
			m_common.m_visibility = visibility;
		}

		root_param_common m_common;
		uint32 m_num_dwords;
	};
	struct root_param_resource final
	{
		enum class e_type : uint8
		{
			srv,
			cbv,
			uav,
			count
		};

		root_param_resource() = default;
		root_param_resource(
			e_type type,
			uint32 shader_register,
			uint32 register_space = 0,
			e_shader_visibility visibility = e_shader_visibility::all)
		{
			m_type = type;
			m_common.m_register_space = register_space;
			m_common.m_shader_register = shader_register;
			m_common.m_visibility = visibility;
		}

		e_type m_type;
		root_param_common m_common;
	};
	struct root_param_resource_range final
	{
		enum class e_type : uint8
		{
			srv,
			cbv,
			uav,
			sampler,
			count
		};

		root_param_resource_range() = default;
		root_param_resource_range(
			uint32 num_resources,
			e_type type,
			uint32 base_shader_register,
			uint32 register_space)
		{
			m_num_resources = num_resources;
			m_type = type;
			m_shader_register = base_shader_register;
			m_register_space = register_space;
		}


		uint32 m_shader_register;
		uint32 m_register_space;

		uint32 m_num_resources;
		e_type m_type;
	};
	struct root_param_resource_table final
	{
		root_param_resource_table() = default;
		root_param_resource_table(const vector<root_param_resource_range>& ranges)
		{
			m_resource_ranges = ranges;
		}
		root_param_resource_table(const root_param_resource_range& range, uint32 sh_reg, uint32 space = 0u, e_shader_visibility vis = e_shader_visibility::all)
		{
			m_common.m_register_space = space;
			m_common.m_shader_register = sh_reg;
			m_resource_ranges.push_back(range);
		}

		root_param_common m_common;
		vector<root_param_resource_range> m_resource_ranges{};
	};
	struct root_static_sampler final
	{
		root_static_sampler() = default;
		root_static_sampler(
			uint32 shader_register,
			uint32 register_space,
			e_shader_visibility visibility,
			float mip_load_bias = 0.0f,
			float min_lod = 0.0f,
			float max_lod = FLT_MAX,
			uint32 max_anisotropy = 16u,
			e_texture_wrap_mode wrap_u = e_texture_wrap_mode::wrap,
			e_texture_wrap_mode wrap_v = e_texture_wrap_mode::wrap,
			e_texture_wrap_mode wrap_w = e_texture_wrap_mode::wrap,
			e_filter filter = e_filter::anisotropic,
			e_comparison_func comp_func = e_comparison_func::lequal,
			e_border_color border_color = e_border_color::white)
			: m_mip_lod_bias{ mip_load_bias }
			, m_min_lod{ min_lod }
			, m_max_lod{ max_lod }
			, m_max_anisotropy{ max_anisotropy }
			, m_wrap_u{ wrap_u }
			, m_wrap_v{ wrap_v }
			, m_wrap_w{ wrap_w }
			, m_filter{ filter }
			, m_comparison_func{ comp_func }
			, m_border_color{ border_color }
		{
			m_common.m_visibility = visibility;
			m_common.m_register_space = register_space;
			m_common.m_shader_register = shader_register;
		}

		root_param_common m_common;

		float m_mip_lod_bias = 0.0f;
		float m_min_lod = 0.0f;
		float m_max_lod = FLT_MAX;
		uint32 m_max_anisotropy = 16u;

		e_texture_wrap_mode m_wrap_u = e_texture_wrap_mode::wrap;
		e_texture_wrap_mode m_wrap_v = e_texture_wrap_mode::wrap;
		e_texture_wrap_mode m_wrap_w = e_texture_wrap_mode::wrap;

		e_filter m_filter = e_filter::anisotropic;
		e_comparison_func m_comparison_func = e_comparison_func::lequal;
		e_border_color m_border_color = e_border_color::white;
	};
	using e_root_range = root_param_resource_range::e_type;
	using e_root_resource = root_param_resource::e_type;
	struct rootsignature_create_args final
	{
		native_device m_device;
		vector<root_param_constants> m_constants;
		vector<root_param_resource> m_resources;
		vector<root_param_resource_table> m_resource_tables;
		vector<root_static_sampler> m_static_samplers;
		umap<string, uint32> m_name_to_register;
		umap<string, uint32> m_name_to_param_idx;
		bool m_direct_indexing = false;

		void reflect_shader(const shader::reflection& reflection, shader::e_shader_type type)
		{
			const e_shader_visibility shader_vis = e_shader_visibility::all;

			for (const shader::reflection::resource& resource : reflection.m_bound_resources)
			{
				if (!resource.m_name.empty())
					m_name_to_register[resource.m_name] = resource.m_shader_register;

				switch (resource.m_type)
				{
				case shader::reflection::resource::e_type::cbv:
					add_root_range(root_param_resource_range::e_type::cbv, resource.m_range_size, resource.m_shader_register, resource.m_register_space, shader_vis);
					name_last_resource_table(resource.m_name);
					break;
				case shader::reflection::resource::e_type::uav:
					add_root_range(root_param_resource_range::e_type::uav, resource.m_range_size, resource.m_shader_register, resource.m_register_space, shader_vis);
					name_last_resource_table(resource.m_name);
					break;
				case shader::reflection::resource::e_type::srv:
				case shader::reflection::resource::e_type::structured:
				case shader::reflection::resource::e_type::texture:
					add_root_range(root_param_resource_range::e_type::srv, resource.m_range_size, resource.m_shader_register, resource.m_register_space, shader_vis);
					name_last_resource_table(resource.m_name);
					break;
				case shader::reflection::resource::e_type::sampler:
					add_root_sampler(resource.m_shader_register, resource.m_register_space, shader_vis);
					name_last_sampler(resource.m_name);
					break;
				}
			}
		}

		// resource ranges are stored in a resource table
		inline void add_root_range(root_param_resource_range::e_type type, uint32 num_resources, uint32 sh_reg, uint32 space = 0u, e_shader_visibility vis = e_shader_visibility::all)
		{
			root_param_resource_range range{ num_resources, type, sh_reg, space };
			root_param_resource_table table{};
			table.m_common.m_register_space = space;
			table.m_common.m_shader_register = sh_reg;
			table.m_common.m_visibility = vis;
			table.m_resource_ranges.push_back(range);
			m_resource_tables.push_back(table);
		}

		// root resources are individual resources not stored in a tables
		inline void add_root_resource(root_param_resource::e_type type, uint32 sh_reg, uint32 space = 0u, e_shader_visibility vis = e_shader_visibility::all)
		{
			m_resources.push_back({ type, sh_reg, space, vis });
		}

		inline void add_root_constants(uint32 num_dwords, uint32 sh_reg, uint32 space = 0u, e_shader_visibility vis = e_shader_visibility::all)
		{
			m_constants.push_back({ num_dwords, sh_reg, space, vis });
		}

		inline void add_root_sampler(
			uint32 shader_register,
			uint32 register_space,
			e_shader_visibility visibility,
			float mip_load_bias = 0.0f,
			float min_lod = 0.0f,
			float max_lod = FLT_MAX,
			uint32 max_anisotropy = 16u,
			e_texture_wrap_mode wrap_u = e_texture_wrap_mode::wrap,
			e_texture_wrap_mode wrap_v = e_texture_wrap_mode::wrap,
			e_texture_wrap_mode wrap_w = e_texture_wrap_mode::wrap,
			e_filter filter = e_filter::anisotropic,
			e_comparison_func comp_func = e_comparison_func::lequal,
			e_border_color border_color = e_border_color::white)
		{
			m_static_samplers.push_back({
				shader_register,
				register_space,
				visibility,
				mip_load_bias,
				min_lod,
				max_lod,
				max_anisotropy,
				wrap_u,
				wrap_v,
				wrap_w,
				filter,
				comp_func,
				border_color });
		}

		/* sets the name of the last constants added */
		inline void name_last_constants(const string& name)
		{
			if (m_constants.empty())
				return;

			m_constants.back().m_common.m_name = name;
		}

		/* sets the name of the last resource added */
		inline void name_last_resource(const string& name)
		{
			if (m_resources.empty())
				return;

			m_resources.back().m_common.m_name = name;
		}

		/* sets the name of the last resource table added */
		inline void name_last_resource_table(const string& name)
		{
			if (m_resource_tables.empty())
				return;

			m_resource_tables.back().m_common.m_name = name;
		}

		/* sets the name of the last sampler added */
		inline void name_last_sampler(const string& name)
		{
			if (m_static_samplers.empty())
				return;

			m_static_samplers.back().m_common.m_name = name;
		}
	};
	struct renderpass_create_args final
	{
		framebuffer_desc m_framebuffer_desc;
		e_renderpass_flags	m_flags = e_renderpass_flags::none;
		color_attachment& set_color(const uint32 index, const texture& texture);
		depth_attachment& set_depth(const texture& texture);
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
		semaphore_create_args,
		buffer_create_args,
		texture_create_args,
		memheap_create_args,
		pipeline_create_args,
		rootsignature_create_args,
		renderpass_create_args>>;

	// =============================================
	// [data_types] this is the extra data associated to each API object
#pragma region data
	struct device_data final
	{
		native_instance		m_instance;
		native_physdevice	m_physical_device;
		uint32				m_descriptor_strides[k_num_descriptor_heap_types];
		queue_families		m_queue_families{};
		uset<object_native> m_children{};

		// these are useful for internally creating rtvs / dsvs on the fly
		native_descheap		m_internal_dsv_heap;
		native_descheap		m_internal_rtv_heap;
	};
	struct queue_data final
	{
		
	};
	struct swapchain_data final
	{
		// internal descriptors
		native_descheap	m_rtv_heap;
		vector<bool>	m_rtv_dirty_list{};
		
		// (vulkan) image acquire info
		struct backbuffer_info final
		{
			bool	m_is_acquired = false;
			uint32	m_current_index = 0u;
		};
		backbuffer_info m_backbuffer_info{};
		vector<texture> m_swapchain_textures{};
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
		native_semaphore	m_semaphore;
		uint32				m_complete_value = 0u;
		e_commandlist_state m_state = e_commandlist_state::init;

		// built on renderpass_begin
		object_native		m_current_framebuffer;
	};
	struct fence_data final
	{

	};
	struct semaphore_data final
	{

	};
	struct buffer_data final
	{
		e_resource_state	m_previous_state;
		e_resource_state	m_current_state;
		uint64				m_bytesize;
		uint64				m_bytestride;
		descriptor			m_buffer_view;
		native_gpu_address	m_gpu_memory_address = {};
	};
	struct texture_data final
	{
		e_resource_state	m_previous_state;
		e_resource_state	m_current_state;
		descriptor			m_texture_view;
		native_gpu_address	m_gpu_memory_address = {};
	};
	struct memheap_data final
	{

	};
	struct pipeline_data final
	{
		native_rootsignature	m_rootsignature;
		object_native			m_vulkan_renderpass;
	};
	struct rootsignature_data final
	{

	};
	struct renderpass_data final
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
		semaphore_data,
		buffer_data,
		texture_data,
		memheap_data,
		pipeline_data,
		rootsignature_data,
		renderpass_data>>;

	template <e_object _t>
	using native_type = std::tuple_element_t < static_cast<uint32>(_t), std::tuple<
		native_device,
		native_queue,
		native_swapchain,
		native_descheap,
		native_commandpool,
		native_commandlist,
		native_fence,
		native_semaphore,
		native_buffer,
		native_texture,
		native_memoryheap,
		native_pipeline,
		native_rootsignature,
		native_renderpass>>;

	template <e_object _t>
	class object
	{
	public:
		using data_type = data_type<_t>;
		using create_args = create_args<_t>;
		using native_obj = native_type<_t>;

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

		native_obj			m_native_object = nullptr;
		create_args			m_create_args = {};
		data_type			m_data = {};
		native_device		m_native_device = nullptr;
	};

	// =============================================
	/* [API class interfaces]
	*	handles to each native API object can be obtained using the 'create_native' functions.
	*	however the point is to use these handles with their implied shared functionality.
	*   that's why this library is centered around using these class interfaces.
	*/

	class renderpass final : public object<e_object::renderpass>
	{
	public:
		using data_type = renderpass_data;
		using create_args = renderpass_create_args;

		inline bool is_colour_target_enabled(const uint32 slot) const
		{ return m_create_args.m_framebuffer_desc.m_color_attachments[slot].m_is_enabled; }

		inline uint32 get_num_colour_targets() const
		{ return m_create_args.m_framebuffer_desc.get_num_enabled_colour_targets(); }

		inline bool is_depth_target_enabled() const
		{ return m_create_args.m_framebuffer_desc.is_depth_enabled(); }

		const framebuffer_desc& get_framebuffer_desc() const
		{ return m_create_args.m_framebuffer_desc; }
	};

	class buffer final : public object<e_object::buffer>
	{
	public:
		using data_type = buffer_data;
		using create_args = buffer_create_args;

		INFLUX_RHI_API result<void*> map_begin(const map_args& args = map_args::full_range());
		INFLUX_RHI_API result<> map_end(const map_args& args);

		template <typename _t, typename _func>
		inline result<> map(_func&& func, const map_args& args = map_args::full_range())
		{
			using result_type = result<>;
			auto map_res = map_begin(args);
			if (!map_res) 
				return result_type::make_error("map_begin() failed!");
			
			func(reinterpret_cast<_t*>(map_res.get()));

			auto map_exit = map_end(args);
			if (!map_exit)
				return result_type::make_error("map_end() failed!");
			return {};
		}

		// uses map() to write the base of the address range (index:0) as _t
		// useful for if your buffer is a single struct (_t)
		template <typename _t>
		inline result<> write_data(const _t& data)
		{
			return map<_t>([&data](_t* target) { (*target) = data; });
		}

		template <typename _t>
		inline result<> write_datas(const vector<_t>& datas)
		{
			return map<_t>([&datas](_t* target)
			{
				for (uint32 i = 0u; i < datas.size(); ++i)
				{
					target[i] = datas[i];
				}
			});
		}

		inline static constexpr e_resource_type get_resource_type() 
		{ return e_resource_type::buffer; };

		inline uint64 get_bytesize() const
		{ return m_create_args.m_bytesize; }

		inline uint64 get_bytestride() const
		{ return m_create_args.m_bytestride; }

		inline uint64 get_num_elements() const
		{ return get_bytesize() / get_bytestride(); }

		inline e_resource_state get_resource_state() const 
		{ return m_data.m_current_state; }
		
		inline e_resource_state get_previous_resource_state() const
		{ return m_data.m_previous_state; }
		
		inline bool allows_uav() const
		{ return has_flag(m_create_args.m_bindflags, e_resource_bindflags::uav); }

		inline result<> set_state(e_resource_state new_state)
		{
			m_data.m_previous_state = m_data.m_current_state;
			m_data.m_current_state = new_state;
			return {};
		}
	};

	class texture final : public object<e_object::texture>
	{
	public:
		using data_type = texture_data;
		using create_args = texture_create_args;

		INFLUX_RHI_API result<> transition(commandlist& cmdlist, e_resource_state new_state);
		INFLUX_RHI_API result<uint64> calculate_bytesize() const;
		INFLUX_RHI_API result<uint64> calculate_bytestride() const;
		INFLUX_RHI_API result<> set_name(const char* name);
		INFLUX_RHI_API result<void*> map_begin(const map_args& args);
		INFLUX_RHI_API result<> map_end();

		inline uint32 get_arraysize() const
		{ return m_create_args.m_arraysize; }
		
		inline uint32 get_depth() const 
		{ return m_create_args.m_dimensions.z; }
		
		inline uint32 get_width() const 
		{ return m_create_args.m_dimensions.x; }
		
		inline uint32 get_height() const 
		{ return m_create_args.m_dimensions.y; }

		inline uint64 get_num_pixels() const
		{ const math::uint3& dim = get_dimensions(); return dim.x * dim.y * dim.z * get_arraysize(); }

		inline math::uint3 get_dimensions() const
		{ return m_create_args.m_dimensions; }
		
		inline uint64 get_bytesize() const 
		{ return calculate_bytesize().get(); }
		
		inline uint64 get_bytestride() const 
		{ return calculate_bytestride().get(); }

		inline uint32 get_num_mips() const
		{ return m_create_args.m_num_mips; }
		
		inline const pixelformat& get_format() const
		{ return m_create_args.m_format; }

		inline const char* get_name() const
		{ return ""; }

		inline static constexpr e_resource_type get_resource_type()
		{ return e_resource_type::texture; }

		inline e_texture_type get_texture_type() const
		{ return m_create_args.m_type; }

		inline e_resource_state get_init_resource_state() const
		{ return m_create_args.m_init_state; }

		inline e_resource_state get_resource_state() const 
		{ return m_data.m_current_state; }

		inline e_resource_state get_previous_resource_state() const
		{ return m_data.m_previous_state; }

		inline bool allows_uav() const
		{ return has_flag(m_create_args.m_bindflags, e_resource_bindflags::uav); }

		inline bool is_valid() const
		{ return object::is_valid(); }

		inline result<> set_state(e_resource_state new_state)
		{
			m_data.m_previous_state = m_data.m_current_state;
			m_data.m_current_state = new_state;
			return {};
		}
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

	class semaphore final : public object<e_object::semaphore>
	{

	};

	class queue final : public object<e_object::queue>
	{
	public:
		inline static queue_create_args default_graphics() { return queue_create_args::default_graphics(); }
		inline static queue_create_args default_compute() { return queue_create_args::default_compute(); }

		INFLUX_RHI_API result<> submit(vector<commandlist*> commandlists) const;
		INFLUX_RHI_API result<> queue_signal(const fence& fence, uint64 signal_value) const;
		INFLUX_RHI_API result<> queue_signal(native_fence fence, uint64 signal_value) const;
	};

	class swapchain final : public object<e_object::swapchain>
	{
	public:
		INFLUX_RHI_API result<> acquire_backbuffer(native_device device);
		INFLUX_RHI_API result<> present(const present_args& args) const;

		INFLUX_RHI_API result<uint32>	get_current_backbuffer_index() const;
		INFLUX_RHI_API result<texture>	get_backbuffer_resource(uint32 index) const;
		INFLUX_RHI_API result<texture>	get_backbuffer_resource() const;
		INFLUX_RHI_API result<>			resize(const math::uint2& new_dim);

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
		INFLUX_RHI_API result<> submit(queue& queue);
		INFLUX_RHI_API result<> renderpass_begin(device& device, renderpass& pass, const begin_renderpass_args& args);
		INFLUX_RHI_API result<> renderpass_end();
		INFLUX_RHI_API result<> dispatch(const math::uint3& group_nums);
		INFLUX_RHI_API result<> clear_texture(device& device, const texture& texture, const clear& clear);

		INFLUX_RHI_API result<> transition(buffer& buffer, e_resource_state new_state);
		INFLUX_RHI_API result<> transition(texture& texture, e_resource_state new_state);

		INFLUX_RHI_API result<> update_blas();
		INFLUX_RHI_API result<> update_tlas();
		INFLUX_RHI_API result<> copy(texture& src, texture& dest);
		INFLUX_RHI_API result<> copy(buffer& src, buffer& dest);

		INFLUX_RHI_API result<> bind_descheaps(descheap const* resource_heap, descheap const* sampler_heap);
		INFLUX_RHI_API result<> bind_rootsignature(const rootsignature& signature);
		INFLUX_RHI_API result<> bind_pipeline(const pipeline& pipeline);
		INFLUX_RHI_API result<> bind_texture_uav(const texture& texture, uint32 param_index);
		INFLUX_RHI_API result<> bind_texture_srv(const texture& texture, uint32 param_index);
		INFLUX_RHI_API result<> bind_buffer_uav(const buffer& buffer, uint32 param_index);
		INFLUX_RHI_API result<> bind_buffer_srv(const buffer& buffer, uint32 param_index);
		INFLUX_RHI_API result<> bind_buffer_cbv(const buffer& buffer, uint32 param_index);
		INFLUX_RHI_API result<> bind_vertexbuffer(const buffer& vertexbuffer);
		INFLUX_RHI_API result<> bind_indexbuffer(const buffer& indexbuffer);

		INFLUX_RHI_API result<> draw(const draw_args& args);
		INFLUX_RHI_API result<> draw_indexed(const draw_indexed_args& args);

		INFLUX_RHI_API result<> set_viewport();
		INFLUX_RHI_API result<> set_xrect();
		INFLUX_RHI_API result<> set_primitive_topology();
		INFLUX_RHI_API result<> end();

		INFLUX_RHI_API result<> wait_for_finish() const;
		INFLUX_RHI_API bool has_fence() const;

		inline bool is_recording() const
		{ return m_data.m_state == e_commandlist_state::recording; }

		inline static commandlist_create_args default_graphics()
		{ return commandlist_create_args::default_graphics(); }

		/* D3D12 ONLY */
		// use clear_texture(device, texture, clear) to get equal result across APIs
		// under the hood, the wrapped D3D12 device will stage its own rtv/dsv view that translates to the resource
#if INFLUX_RHI_D3D12
		INFLUX_RHI_API result<> clear_rtv(descriptor rtv, const clear& clear);
		INFLUX_RHI_API result<> clear_dsv(descriptor dsv);
		INFLUX_RHI_API result<> set_draw_output(descriptor rtv, descriptor dsv);
#endif
	};

	class commandpool final : public object<e_object::commandpool>
	{

	};

	class pipeline final : public object<e_object::pipeline>
	{
	public:
		inline uint32 get_num_colour_targets() const
		{ return m_create_args.m_graphics.m_output_merger.get_num_enabled_rendertargets(); }

		inline uint32 is_depth_target_enabled() const
		{ return m_create_args.m_graphics.m_output_merger.m_depthtarget.m_depth_enable; }

		e_pipeline_type get_type() const
		{ return m_create_args.m_type; }

		const rasterizer& get_rasterizer() const
		{ return m_create_args.m_graphics.m_rasterizer; }

		const output_merger& get_output_merger() const
		{ return m_create_args.m_graphics.m_output_merger; }
	};
	
	class rootsignature final : public object<e_object::rootsignature>
	{

	};

	/*
	*	device has a bunch of create() functions.
	*   these are helpers that essentially call create_native(obj) AND
	*		- store their out_data in the wrapper object
	*		- register their handle to this owning device (for later cleanup)
	*	
	*	using this API, use the device to create & clean up your resources!
	*/
	class device final : public object<e_object::device>
	{
	public:
		using create_args = device_create_args;
		using data_type = device_data;

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
		inline result<fence>			create(const fence_create_args& args);
		inline result<semaphore>		create(const semaphore_create_args& args);
		inline result<texture>			create(const texture_create_args& args);
		inline result<buffer>			create(const buffer_create_args& args);
		inline result<renderpass>		create(const renderpass_create_args& args);

		inline result<pipeline>			create_graphics_pipeline(
			const rootsignature& signature,
			const renderpass& renderpass,
			const graphics_shaderslots& shaders, 
			const graphics_pipeline_desc& desc)
		{
			pipeline_create_args args{};
			args.m_type = e_pipeline_type::graphics;
			args.m_graphics = desc;
			args.m_graphics_shaders = shaders;
			args.m_rootsignature = signature.m_native_object;
			args.m_renderpass = renderpass.m_native_object;
			return create(args);
		}

		inline result<> release()
		{
			return {};
		}

	private:
		template <typename _obj>
		inline result<> register_child(_obj& obj)
		{
			using result_type = result<>;
			obj.m_native_device = m_native_object;

			if (obj.m_native_object == nullptr)
				return result_type::make_error("cannot register nullptr!");

			if (m_data.m_children.contains(obj.m_native_object))
				return result_type::make_error("native object already registered!");

			m_data.m_children.insert(obj.m_native_object);
			return {};
		}
	};

	inline static result<device> create_device(const device_create_args& args = {})
	{
		return device::create(args);
	}

	// =============================================
	/* [import methods] (DX12 only)
	* Dx12 handles store their meta-data (GetDesc())
	* this API can import an object (native pointer) and attempt to build a corresponding wrapped type.
	* based on what information it can parse from the pointer.
	* 
	* Vulkan does not store object metadata in it's handles.
	* That makes this part of the API not supported for Vulkan.
	*/
#if !INFLUX_RHI_VULKAN
	INFLUX_RHI_API result<buffer>			import_buffer(native_buffer native);
	INFLUX_RHI_API result<texture>			import_texture(native_texture native);
	INFLUX_RHI_API result<descheap>			import_descheap(native_descheap native);
	INFLUX_RHI_API result<commandlist>		import_commandlist(native_commandlist native);
	INFLUX_RHI_API result<commandpool>		import_commandpool(native_commandpool native);
#endif

	// =============================================
	/* [creation methods]
	* these are the platform-native object creation methods
	* if you want to work closer to the API, you can just use these to create your objects
	* then cast them to the type you expect them to be...
	* 
	* optionally, you can specify an address 'out_data' to which the create function writes various queried information
	*/ 
	INFLUX_RHI_API result<native_device>			create_native(const device_create_args& args, device_data* out_data = nullptr);
	INFLUX_RHI_API result<native_queue>				create_native(const queue_create_args& args, queue_data* out_data = nullptr);
	INFLUX_RHI_API result<native_swapchain>			create_native(const swapchain_create_args& args, swapchain_data* out_data = nullptr);
	INFLUX_RHI_API result<native_descheap>			create_native(const descheap_create_args& args, descheap_data* out_data = nullptr);
	INFLUX_RHI_API result<native_commandpool>		create_native(const commandpool_create_args& args, commandpool_data* out_data = nullptr);
	INFLUX_RHI_API result<native_commandlist>		create_native(const commandlist_create_args& args, commandlist_data* out_data = nullptr);
	INFLUX_RHI_API result<native_fence>				create_native(const fence_create_args& args, fence_data* out_data = nullptr);
	INFLUX_RHI_API result<native_semaphore>			create_native(const semaphore_create_args& args, semaphore_data* out_data = nullptr);
	INFLUX_RHI_API result<native_buffer>			create_native(const buffer_create_args& args, buffer_data* out_data = nullptr);
	INFLUX_RHI_API result<native_texture>			create_native(const texture_create_args& args, texture_data* out_data = nullptr);
	INFLUX_RHI_API result<native_pipeline>			create_native(const pipeline_create_args& args, pipeline_data* out_data = nullptr);
	INFLUX_RHI_API result<native_rootsignature>		create_native(const rootsignature_create_args& args, rootsignature_data* out_data = nullptr);
	INFLUX_RHI_API result<native_renderpass>		create_native(const renderpass_create_args& args, renderpass_data* out_data = nullptr);
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
		args_cpy.m_queue_families = m_data.m_queue_families;
		auto res = influx::rhi::create<swapchain>(args_cpy);
		if (!res)
			return result_type::make_error("failed creating swapchain!");
		
		auto reg = register_child(res.get());
		if (!reg)
			return result_type::make_error("failed registering new object!");
		return res;
	}
	inline result<queue> device::create(const queue_create_args& args)				
	{ 
		using result_type = result<queue>;

		auto args_cpy = args; 
		args_cpy.m_queue_families = m_data.m_queue_families;
		args_cpy.m_device = (native_device)this->m_native_object; 
		auto res = influx::rhi::create<queue>(args_cpy);
		if (!res)
			return result_type::make_error("failed creating queue!");

		auto reg = register_child(res.get());
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

		auto reg = register_child(res.get());
		if (!reg)
			return result_type::make_error("failed registering new object!");
		return res;
	}
	inline result<commandlist> device::create(const commandlist_create_args& args)		
	{ 
		using result_type = result<commandlist>;

		auto args_cpy = args; 
		args_cpy.m_device = (native_device)this->m_native_object;
		args_cpy.m_queue_families = m_data.m_queue_families;
		auto res = influx::rhi::create<commandlist>(args_cpy);
		if (!res)
			return result_type::make_error("failed creating commandlist!");

		auto reg = register_child(res.get());
		if (!reg)
			return result_type::make_error("failed registering new object!");
		return res;
	}
	inline result<descheap>	device::create(const descheap_create_args& args)			
	{ 
		using result_type = result<descheap>;

		auto args_cpy = args; 
		args_cpy.m_device = (native_device)this->m_native_object;
		auto res = influx::rhi::create<descheap>(args_cpy); 
			return result_type::make_error("failed creating descheap!");

		auto reg = register_child(res.get());
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
		if (!res)
			return result_type::make_error("failed creating pipeline!");

		auto reg = register_child(res.get());
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

		auto reg = register_child(res.get());
		if (!reg)
			return result_type::make_error("failed registering new object!");
		return res;
	}
	inline result<fence> device::create(const fence_create_args& args)
	{
		using result_type = result<fence>;
		auto args_cpy = args;
		args_cpy.m_device = (native_device)this->m_native_object;
		auto res = influx::rhi::create<fence>(args_cpy);
		if (!res)
			return result_type::make_error("failed creating fence!");

		auto reg = register_child(res.get());
		if (!reg)
			return result_type::make_error("failed registering new object!");
		return res;
	}
	inline result<semaphore> device::create(const semaphore_create_args& args)
	{
		using result_type = result<semaphore>;
		auto args_cpy = args;
		args_cpy.m_device = (native_device)this->m_native_object;
		auto res = influx::rhi::create<semaphore>(args_cpy);
		if (!res)
			return result_type::make_error("failed creating semaphore!");

		auto reg = register_child(res.get());
		if (!reg)
			return result_type::make_error("failed registering new object!");
		return res;
	}
	inline result<texture> device::create(const texture_create_args& args)
	{
		using result_type = result<texture>;
		auto args_cpy = args;
		args_cpy.m_device = (native_device)this->m_native_object;
		args_cpy.m_physdevice = (native_physdevice)this->m_data.m_physical_device;
		auto res = influx::rhi::create<texture>(args_cpy);
		if (!res)
			return result_type::make_error("failed creating texture!");

		auto reg = register_child(res.get());
		if (!reg)
			return result_type::make_error("failed registering new object!");
		return res;
	}
	inline result<buffer> device::create(const buffer_create_args& args)
	{
		using result_type = result<buffer>;
		auto args_cpy = args;
		args_cpy.m_device = (native_device)this->m_native_object;
		args_cpy.m_physdevice = (native_physdevice)this->m_data.m_physical_device;
		auto res = influx::rhi::create<buffer>(args_cpy);
		if (!res)
			return result_type::make_error("failed creating buffer!");

		auto reg = register_child(res.get());
		if (!reg)
			return result_type::make_error("failed registering new object!");
		return res;
	}
	inline result<renderpass> device::create(const renderpass_create_args& args)
	{
		using result_type = result<renderpass>;
		auto args_cpy = args;
		args_cpy.m_device = (native_device)this->m_native_object;
		auto res = influx::rhi::create<renderpass>(args_cpy);
		if (!res)
			return result_type::make_error("failed creating renderpass!");

		auto reg = register_child(res.get());
		if (!reg)
			return result_type::make_error("failed registering new object!");
		return res;
	}

	inline color_attachment& renderpass_create_args::set_color(const uint32 index, const texture& texture)
	{
		color_attachment new_attach{};
		new_attach.m_store;
		new_attach.m_load;
		new_attach.m_format = texture.get_format();
		
		if (m_framebuffer_desc.m_dimensions.is_zero())
		{
			m_framebuffer_desc.m_dimensions.x = texture.get_dimensions().x;
			m_framebuffer_desc.m_dimensions.y = texture.get_dimensions().y;
		}

		color_attachment& target = m_framebuffer_desc.m_color_attachments[index];
		target = new_attach;
		target.m_is_enabled = true;
		return target;
	}
	inline depth_attachment& renderpass_create_args::set_depth(const texture& texture)
	{
		depth_attachment new_attach{};
		new_attach.m_format = texture.get_format();
		new_attach.m_is_enabled = true;

		if (m_framebuffer_desc.m_dimensions.is_zero())
		{
			m_framebuffer_desc.m_dimensions.x = texture.get_dimensions().x;
			m_framebuffer_desc.m_dimensions.y = texture.get_dimensions().y;
		}

		depth_attachment& target = m_framebuffer_desc.m_depth_attachment;
		target = new_attach;
		target.m_is_enabled = true;
		return target;
	}
	inline void begin_renderpass_args::set_color(const uint32 index, const texture& texture)
	{
		m_color_targets[index] = &texture;
	}
	inline void begin_renderpass_args::set_depth(const texture& texture)
	{
		m_depth_target = &texture;
	}
}
ENABLE_ENUM_BIT_OPERATORS(influx::rhi::e_resource_state);
ENABLE_ENUM_BIT_OPERATORS(influx::rhi::e_resource_bindflags);
ENABLE_ENUM_BIT_OPERATORS(influx::rhi::e_graphics_shader_pipeline);
ENABLE_ENUM_BIT_OPERATORS(influx::rhi::e_queue_flags);
ENABLE_ENUM_BIT_OPERATORS(influx::rhi::e_memoryheap_flags);

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