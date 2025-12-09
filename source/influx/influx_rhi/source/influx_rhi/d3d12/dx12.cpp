#include "rhi_pch.h"
#include "influx_rhi.h"

#if INFLUX_RHI_D3D12
#include "d3d12.h"
#include "dxgi1_6.h"
#include "d3dx12/d3dx12.h"
#include <D3Dcompiler.h>

namespace influx::rhi
{
	using dx12_instance		= IDXGIFactory2;
	using dx12_factory		= IDXGIFactory2;
	using dx12_physdevice	= IDXGIAdapter1;
	using dx12_queue		= ID3D12CommandQueue;
	using dx12_swapchain	= IDXGISwapChain4;
	using dx12_device		= ID3D12Device;
	using dx12_memheap		= ID3D12Heap;
	using dx12_resource		= ID3D12Resource;
	using dx12_commandlist	= ID3D12GraphicsCommandList;
	using dx12_allocator	= ID3D12CommandAllocator;
	using dx12_descheap		= ID3D12DescriptorHeap;
	using dx12_fence		= ID3D12Fence;
	using dx12_renderpass	= ID3D12Fence;
	using dx12_pipeline		= ID3D12PipelineState;
	using dx12_rootsignature = ID3D12RootSignature;

	template <typename _t, typename _p>
	inline result<_t*> cast(_p* ptr)
	{
		if (ptr == nullptr)
			return result<_t*>::make_error("cannot cast when ptr is nullptr!");

		_t* res = (_t*)ptr;
		if (res) return res;
		else return result<_t*>::make_error("failed casting ptr to type!");
	}

	// [helpers]
#pragma region helpers
	inline string hres_to_string(HRESULT hr)
	{
		char* msgBuf = nullptr;

		DWORD size = FormatMessageA(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			nullptr,
			hr,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPSTR)&msgBuf,
			0,
			nullptr);

		string result = (size && msgBuf) ? msgBuf : "Unknown error";
		LocalFree(msgBuf);
		return result;
	}
	template <class _t = char>
	inline result<_t> hres_to_result(HRESULT hres, const _t& value_if_success)
	{
		using result_type = result<_t>;
		if (SUCCEEDED(hres) == false)
			return result_type::make_error(hres_to_string(hres).c_str());

		return value_if_success;
	}

	D3D12_COMMAND_LIST_TYPE translate(e_queue_type type)
	{
		switch (type)
		{
		default:
		case e_queue_type::graphics: return D3D12_COMMAND_LIST_TYPE_DIRECT;
		}
	}
	D3D12_COMMAND_LIST_TYPE translate(e_commandlist_type type)
	{
		switch (type)
		{
		default:
		case e_commandlist_type::graphics: return D3D12_COMMAND_LIST_TYPE_DIRECT;
		}
	}
	D3D12_DESCRIPTOR_HEAP_TYPE translate(e_descriptor_heap_type type)
	{
		switch (type)
		{
		case e_descriptor_heap_type::rtv: return D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		case e_descriptor_heap_type::dsv: return D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		case e_descriptor_heap_type::rsc: return D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		case e_descriptor_heap_type::sampler: return D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
		}
		return {};
	}
	D3D12_RESOURCE_STATES translate(e_resource_state state)
	{
		D3D12_RESOURCE_STATES result{};
		if (has_flag(state, e_resource_state::common))			result |= D3D12_RESOURCE_STATE_COMMON;
		if (has_flag(state, e_resource_state::copy_src))		result |= D3D12_RESOURCE_STATE_COPY_SOURCE;
		if (has_flag(state, e_resource_state::copy_dst))		result |= D3D12_RESOURCE_STATE_COPY_DEST;
		if (has_flag(state, e_resource_state::render_target))	result |= D3D12_RESOURCE_STATE_RENDER_TARGET;
		if (has_flag(state, e_resource_state::depth_target))	result |= D3D12_RESOURCE_STATE_DEPTH_WRITE;
		if (has_flag(state, e_resource_state::present))			result |= D3D12_RESOURCE_STATE_PRESENT;
		return result;
	}
	D3D12_RESOURCE_FLAGS translate(e_resource_bindflags flags)
	{
		D3D12_RESOURCE_FLAGS result = D3D12_RESOURCE_FLAG_NONE;
		if (has_flag(flags, e_resource_bindflags::rtv)) result |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
		if (has_flag(flags, e_resource_bindflags::dsv)) result |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
		if (has_flag(flags, e_resource_bindflags::uav)) result |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		if (has_flag(flags, e_resource_bindflags::srv)) result |= D3D12_RESOURCE_FLAG_ALLOW_SIMULTANEOUS_ACCESS;
		return result;
	}
	e_descriptor_heap_type translate(D3D12_DESCRIPTOR_HEAP_TYPE type)
	{
		switch (type)
		{
		case D3D12_DESCRIPTOR_HEAP_TYPE_RTV: return e_descriptor_heap_type::rtv;
		case D3D12_DESCRIPTOR_HEAP_TYPE_DSV: return e_descriptor_heap_type::dsv;
		case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV: return e_descriptor_heap_type::rsc;
		case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER: return e_descriptor_heap_type::sampler;
		}
		return e_descriptor_heap_type::num;
	}
	e_resource_type translate(D3D12_RESOURCE_DIMENSION type)
	{
		switch (type)
		{
		default:
		case D3D12_RESOURCE_DIMENSION_UNKNOWN: return {};
		case D3D12_RESOURCE_DIMENSION_BUFFER: return e_resource_type::buffer;
		case D3D12_RESOURCE_DIMENSION_TEXTURE1D: return e_resource_type::texture;
		case D3D12_RESOURCE_DIMENSION_TEXTURE2D: return e_resource_type::texture;
		case D3D12_RESOURCE_DIMENSION_TEXTURE3D: return e_resource_type::texture;
		}
	}
	D3D12_DSV_DIMENSION translate_dsv(e_texture_type type, uint32 array_num)
	{
		if (array_num > 1u)
		{
			switch (type)
			{
			case e_texture_type::texture1D: return D3D12_DSV_DIMENSION_TEXTURE1DARRAY;
			case e_texture_type::texture2D: return D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
			}
		}
		else
		{
			switch (type)
			{
			case e_texture_type::texture1D: return D3D12_DSV_DIMENSION_TEXTURE1D;
			case e_texture_type::texture2D: return D3D12_DSV_DIMENSION_TEXTURE2D;
			}
		}
		return D3D12_DSV_DIMENSION_UNKNOWN;
	}
	D3D12_RTV_DIMENSION translate_rtv(e_texture_type type, uint32 array_num)
	{
		if (array_num > 1u)
		{
			switch (type)
			{
			case e_texture_type::texture1D: return D3D12_RTV_DIMENSION_TEXTURE1DARRAY;
			case e_texture_type::texture2D: return D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
			}
		}
		else
		{
			switch (type)
			{
			case e_texture_type::texture1D: return D3D12_RTV_DIMENSION_TEXTURE1D;
			case e_texture_type::texture2D: return D3D12_RTV_DIMENSION_TEXTURE2D;
			case e_texture_type::texture3D: return D3D12_RTV_DIMENSION_TEXTURE3D;
			}
		}
		return D3D12_RTV_DIMENSION_UNKNOWN;
	}
	D3D12_RESOURCE_DIMENSION translate(e_resource_type res_type, e_texture_type tex_type = e_texture_type::num)
	{
		switch (res_type)
		{
		case e_resource_type::texture:
			switch (tex_type)
			{
				case e_texture_type::texture1D: return D3D12_RESOURCE_DIMENSION_TEXTURE1D;
				case e_texture_type::texture2D: return D3D12_RESOURCE_DIMENSION_TEXTURE2D;
				case e_texture_type::texture3D: return D3D12_RESOURCE_DIMENSION_TEXTURE3D;
			}
			break;
		case e_resource_type::buffer:
			return D3D12_RESOURCE_DIMENSION_BUFFER;
		}
		return D3D12_RESOURCE_DIMENSION_UNKNOWN;
	}
	constexpr D3D12_RENDER_PASS_FLAGS translate(e_renderpass_flags flags)
	{
		D3D12_RENDER_PASS_FLAGS translated = D3D12_RENDER_PASS_FLAG_NONE;
		if (flags & e_renderpass_flags::read_only_depth) translated |= D3D12_RENDER_PASS_FLAG_BIND_READ_ONLY_DEPTH;
		if (flags & e_renderpass_flags::read_only_stencil) translated |= D3D12_RENDER_PASS_FLAG_BIND_READ_ONLY_STENCIL;
		if (flags & e_renderpass_flags::allow_uav_write) translated |= D3D12_RENDER_PASS_FLAG_ALLOW_UAV_WRITES;
		if (flags & e_renderpass_flags::suspending) translated |= D3D12_RENDER_PASS_FLAG_SUSPENDING_PASS;
		if (flags & e_renderpass_flags::resuming) translated |= D3D12_RENDER_PASS_FLAG_RESUMING_PASS;
		return translated;
	}
	constexpr D3D12_RENDER_PASS_BEGINNING_ACCESS translate(e_load_op load_op)
	{
		D3D12_RENDER_PASS_BEGINNING_ACCESS access{};
		switch (load_op)
		{
		case e_load_op::discard: access.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_DISCARD; break;
		case e_load_op::preserve: access.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_PRESERVE; break;
		case e_load_op::clear: access.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_CLEAR; break;
		case e_load_op::no_access: access.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_NO_ACCESS; break;
		}

		return access;
	}
	constexpr D3D12_RENDER_PASS_ENDING_ACCESS translate(e_store_op store_op)
	{
		D3D12_RENDER_PASS_ENDING_ACCESS access{};
		switch (store_op)
		{
		case e_store_op::discard: access.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_DISCARD; break;
		case e_store_op::preserve: access.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE; break;
		case e_store_op::resolve: access.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_RESOLVE; break;
		case e_store_op::no_access: access.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_NO_ACCESS; break;
		}
		return access;
	}
	inline D3D12_SHADER_VISIBILITY translate(e_shader_visibility vis)
	{
		uint32 result{};

		if (vis == e_shader_visibility::all)
			return D3D12_SHADER_VISIBILITY_ALL;

		if (has_flag(vis, e_shader_visibility::vertex))		result |= (uint32)D3D12_SHADER_VISIBILITY_VERTEX;
		if (has_flag(vis, e_shader_visibility::pixel))		result |= (uint32)D3D12_SHADER_VISIBILITY_PIXEL;
		if (has_flag(vis, e_shader_visibility::domain))		result |= (uint32)D3D12_SHADER_VISIBILITY_DOMAIN;
		if (has_flag(vis, e_shader_visibility::hull))		result |= (uint32)D3D12_SHADER_VISIBILITY_HULL;
		if (has_flag(vis, e_shader_visibility::geometry))	result |= (uint32)D3D12_SHADER_VISIBILITY_GEOMETRY;
		if (has_flag(vis, e_shader_visibility::compute))	result |= (uint32)D3D12_SHADER_VISIBILITY_ALL;
		if (has_flag(vis, e_shader_visibility::amp))		result |= (uint32)D3D12_SHADER_VISIBILITY_AMPLIFICATION;
		if (has_flag(vis, e_shader_visibility::mesh))		result |= (uint32)D3D12_SHADER_VISIBILITY_MESH;
		return (D3D12_SHADER_VISIBILITY)result;
	}
	inline D3D12_TEXTURE_ADDRESS_MODE translate(e_texture_wrap_mode wrap)
	{
		switch (wrap)
		{
		case e_texture_wrap_mode::wrap: return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		case e_texture_wrap_mode::border: return D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		case e_texture_wrap_mode::mirror: return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
		case e_texture_wrap_mode::mirror_once: return D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE;
		case e_texture_wrap_mode::clamp: return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		default:
		case e_texture_wrap_mode::count:
			influx_assert(false);
			return D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		}
	}
	inline D3D12_STATIC_BORDER_COLOR translate(e_border_color color)
	{
		switch (color)
		{
		case e_border_color::white: return D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
		case e_border_color::black: return D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
		case e_border_color::black_transparent: return D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
		default:
		case e_border_color::count:
			return D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
		}
	}
	inline D3D12_COMPARISON_FUNC translate(e_comparison_func func)
	{
		switch (func)
		{
		case e_comparison_func::lequal: return D3D12_COMPARISON_FUNC_LESS_EQUAL;
		case e_comparison_func::always: return D3D12_COMPARISON_FUNC_ALWAYS;
		case e_comparison_func::less: return D3D12_COMPARISON_FUNC_LESS;
		case e_comparison_func::gequal: return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
		case e_comparison_func::greater: return D3D12_COMPARISON_FUNC_GREATER;
		default:
		case e_comparison_func::count:
			influx_assert(false);
			return D3D12_COMPARISON_FUNC_LESS_EQUAL;
		}
	}
	inline D3D12_FILTER translate(e_filter filter)
	{
		return (D3D12_FILTER)filter;
	}
	inline D3D12_CULL_MODE translate(e_cull_mode mode)
	{
		switch (mode)
		{
		case e_cull_mode::back: return D3D12_CULL_MODE_BACK;
		case e_cull_mode::front: return D3D12_CULL_MODE_FRONT;
		case e_cull_mode::nocull: return D3D12_CULL_MODE_NONE;
		default:
		case e_cull_mode::count: return D3D12_CULL_MODE_NONE;
		}
	}
	inline D3D12_FILL_MODE translate(e_fill_mode mode)
	{
		switch (mode)
		{
		case e_fill_mode::wireframe: return D3D12_FILL_MODE_WIREFRAME;
		case e_fill_mode::solid: return D3D12_FILL_MODE_SOLID;
		default:
		case e_fill_mode::count:
			influx_assert(false);
			return  D3D12_FILL_MODE_SOLID;
		}
	}
	inline D3D12_BLEND translate(e_blend blend)
	{
		return (D3D12_BLEND)blend;
	}
	inline D3D12_BLEND_OP translate(e_blendop op)
	{
		return (D3D12_BLEND_OP)op;
	}
	static D3D12_RASTERIZER_DESC translate(const rasterizer& desc)
	{
		CD3DX12_RASTERIZER_DESC rasterizer_desc(D3D12_DEFAULT);
		rasterizer_desc.CullMode = translate(desc.m_cullmode);
		rasterizer_desc.FillMode = translate(desc.m_fillmode);
		rasterizer_desc.MultisampleEnable = desc.m_multisample;
		rasterizer_desc.FrontCounterClockwise = desc.m_front_ccw;
		rasterizer_desc.DepthBias = desc.m_depth_bias;
		rasterizer_desc.DepthBiasClamp = desc.m_depth_bias_clamp;
		rasterizer_desc.SlopeScaledDepthBias = desc.m_slope_depth_bias;
		rasterizer_desc.DepthClipEnable = desc.m_depth_clip_enable;
		rasterizer_desc.AntialiasedLineEnable = desc.m_antialiased_line;
		rasterizer_desc.ForcedSampleCount = desc.m_forced_samplecount;
		rasterizer_desc.ConservativeRaster = desc.m_conservative ? D3D12_CONSERVATIVE_RASTERIZATION_MODE_ON : D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
		return rasterizer_desc;
	}
	static D3D12_BLEND_DESC translate(
		const blend_desc* desc, uint32 num_blends,
		bool is_alpha_to_coverage_enabled)
	{
		CD3DX12_BLEND_DESC blend_desc(D3D12_DEFAULT);
		bool has_multiple_blends = false;
		for (size_t i = 0u; i < num_blends; ++i)
		{
			if (i > 0u) has_multiple_blends |= desc[i].m_enabled;
			blend_desc.RenderTarget[i].BlendEnable = desc[i].m_enabled;
			blend_desc.RenderTarget[i].SrcBlend = translate(desc[i].m_src);
			blend_desc.RenderTarget[i].DestBlend = translate(desc[i].m_dest);
			blend_desc.RenderTarget[i].BlendOp = translate(desc[i].m_op);
			blend_desc.RenderTarget[i].SrcBlendAlpha = translate(desc[i].m_srcalpha);
			blend_desc.RenderTarget[i].DestBlendAlpha = translate(desc[i].m_destalpha);
			blend_desc.RenderTarget[i].BlendOpAlpha = translate(desc[i].m_op_alpha);
			blend_desc.RenderTarget[i].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL; // desc.m_blends[i].m_write_mask;
		}
		blend_desc.AlphaToCoverageEnable = is_alpha_to_coverage_enabled;
		blend_desc.IndependentBlendEnable = has_multiple_blends; // implicit independent blend
		return blend_desc;
	}
	static D3D12_DEPTH_STENCIL_DESC translate(const output_merger::per_depthtarget& desc)
	{
		CD3DX12_DEPTH_STENCIL_DESC depth_stencil_desc(D3D12_DEFAULT);
		depth_stencil_desc.DepthEnable = desc.m_depth_enable;
		depth_stencil_desc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
		depth_stencil_desc.DepthFunc = translate(desc.m_depth_func);
		depth_stencil_desc.StencilEnable = desc.m_stencil_enable;
		return depth_stencil_desc;
	}
	static D3D12_PRIMITIVE_TOPOLOGY_TYPE translate(e_primitive_topology_type type)
	{
		switch (type)
		{
		case e_primitive_topology_type::triangle:	return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		case e_primitive_topology_type::line:		return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
		case e_primitive_topology_type::patch:		return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
		case e_primitive_topology_type::point:		return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
		default:
		case e_primitive_topology_type::count:
			influx_assert(false);
			return D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
		}
	}
	static D3D12_PRIMITIVE_TOPOLOGY_TYPE translate_type(e_primitive_topology type)
	{
		switch (type)
		{
		case e_primitive_topology::trilist:	return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		case e_primitive_topology::linelist: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
		default:
			influx_assert(false);
			return D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
		}
	}
	static D3D_PRIMITIVE_TOPOLOGY translate(e_primitive_topology topo)
	{
		switch (topo)
		{
		case e_primitive_topology::trilist: return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		case e_primitive_topology::linelist: return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
		default:
		case e_primitive_topology::count:
			influx_assert(false);
			return D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
		}
	}
	inline D3D12_LOGIC_OP translate(e_logic_op operation)
	{
		switch (operation)
		{
			case e_logic_op::clear		: return D3D12_LOGIC_OP_CLEAR;
			case e_logic_op::set		: return D3D12_LOGIC_OP_SET;
			case e_logic_op::copy		: return D3D12_LOGIC_OP_COPY;
			case e_logic_op::copy_inv	: return D3D12_LOGIC_OP_COPY_INVERTED;
			case e_logic_op::noop		: return D3D12_LOGIC_OP_NOOP;
			case e_logic_op::invert		: return D3D12_LOGIC_OP_INVERT;
			case e_logic_op::AND		: return D3D12_LOGIC_OP_AND;
			case e_logic_op::NAND		: return D3D12_LOGIC_OP_NAND;
			case e_logic_op::OR			: return D3D12_LOGIC_OP_OR;
			case e_logic_op::NOR		: return D3D12_LOGIC_OP_NOR;
			case e_logic_op::XOR		: return D3D12_LOGIC_OP_XOR;
			case e_logic_op::EQUIV		: return D3D12_LOGIC_OP_EQUIV;
			case e_logic_op::AND_REV	: return D3D12_LOGIC_OP_AND_REVERSE;
			case e_logic_op::AND_INV	: return D3D12_LOGIC_OP_AND_INVERTED;
			case e_logic_op::OR_REV		: return D3D12_LOGIC_OP_OR_REVERSE;
			case e_logic_op::OR_INV		: return D3D12_LOGIC_OP_INVERT;
		}
		return D3D12_LOGIC_OP_NOOP;
	}

	static constexpr uint32 k_num_dxgi_formats = 256u;
	class static_pixel_formats final
	{
	private:
		static bool m_initialized;
		static std::pair<const char*, pixelformat> m_formats[k_num_dxgi_formats];
		static uint32 m_num_supported_formats;

		static void initialize();

	public:
		static uint32 get_num_supported_formats()
		{
			if (!m_initialized) initialize();
			return m_num_supported_formats;
		}

		static bool is_supported(const pixelformat& format)
		{
			for (uint32 i = 0u; i < m_num_supported_formats; ++i)
				if (m_formats[i].second == format) return true;
			return false;
		}

		static std::pair<const char*, pixelformat> const* get_formats()
		{
			if (!m_initialized) initialize();
			return m_formats;
		}
	};
	bool static_pixel_formats::m_initialized = false;
	uint32 static_pixel_formats::m_num_supported_formats = 0u;
	std::pair<const char*, pixelformat> static_pixel_formats::m_formats[k_num_dxgi_formats]{};

	uint32 get_num_supported_pixel_formats()
	{
		return static_pixel_formats::get_num_supported_formats();
	}
	bool is_format_supported(const pixelformat& format)
	{
		return static_pixel_formats::is_supported(format);
	}
	DXGI_FORMAT translate(const pixelformat& format);

	uint32 get_translated_pixelformat(const pixelformat& format)
	{
		return translate(format);
	}
	const char* get_pixelformat_string(const pixelformat& format)
	{
		return static_pixel_formats::get_formats()[translate(format)].first;
	}
	const pixelformat& translate(DXGI_FORMAT format)
	{
		return static_pixel_formats::get_formats()[format].second;
	}
	DXGI_FORMAT translate(const pixelformat& format)
	{
		using namespace format;
		for (uint32 i = 0u; i < k_num_dxgi_formats; ++i)
		{
			DXGI_FORMAT as_dxgi = (DXGI_FORMAT)i;
			const pixelformat& form = translate(as_dxgi);
			if (format == form)
			{
				return as_dxgi;
			}
		}
		return DXGI_FORMAT_UNKNOWN;
	}
	DXGI_FORMAT translate(const bufferformat& format)
	{
		using namespace format;
		for (uint32 i = 0u; i < k_num_dxgi_formats; ++i)
		{
			DXGI_FORMAT as_dxgi = (DXGI_FORMAT)i;
			const pixelformat& form = translate(as_dxgi);
			if (format == form)
			{
				return as_dxgi;
			}
		}
		return DXGI_FORMAT_UNKNOWN;
	}
	result<uint32> query_descriptor_stride(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type)
	{
		return device->GetDescriptorHandleIncrementSize(type);
	}
	result<descriptor> sample_descheap(ID3D12DescriptorHeap* heap, uint32 stride, uint32 index, bool cpu)
	{
		if (cpu)
		{
			D3D12_CPU_DESCRIPTOR_HANDLE base = heap->GetCPUDescriptorHandleForHeapStart();
			base.ptr += (index * stride);
			return static_cast<descriptor>(base.ptr);
		}
		else
		{
			D3D12_GPU_DESCRIPTOR_HANDLE base = heap->GetGPUDescriptorHandleForHeapStart();
			base.ptr += (index * stride);
			return static_cast<descriptor>(base.ptr);
		}
	}
	result<descriptor> sample_descheap(ID3D12Device* device, ID3D12DescriptorHeap* heap, uint32 index, bool cpu)
	{
		uint32 stride = query_descriptor_stride(device, heap->GetDesc().Type).get();
		return sample_descheap(heap, stride, index, cpu);
	}
#pragma endregion

	result<native_device> create_native(const device_create_args& args, device_data* out_data)
	{
		using result_type = result<native_device>;

		// setup a dxgi factory
		dx12_factory* dxfactory = nullptr;
		HRESULT hres = ::CreateDXGIFactory2(0u, IID_PPV_ARGS(&dxfactory));
		auto create_factory_res = hres_to_result<dx12_factory*>(hres, dxfactory);
		if (!create_factory_res)
			return result_type::make_error("failed creating DXGI factory!");

		// if no physical device was specified,
		// find a physical device from querying..
		device_create_args edited_args = args;
		if (!args.m_physdevice.has_value())
		{
			vector<dx12_physdevice*> physical_devices{};
			constexpr bool prefer_performance = true;

			IDXGIAdapter1* adapter = nullptr;
			IDXGIFactory6* factory6;
			if (SUCCEEDED(dxfactory->QueryInterface(IID_PPV_ARGS(&factory6))))
			{
				for (UINT adapterIndex = 0;
					SUCCEEDED(factory6->EnumAdapterByGpuPreference(
						adapterIndex,
						prefer_performance ? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE : DXGI_GPU_PREFERENCE_UNSPECIFIED,
						IID_PPV_ARGS(&adapter)));
						++adapterIndex)
				{
					DXGI_ADAPTER_DESC1 desc;
					adapter->GetDesc1(&desc);

					const bool adapter_is_software = desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE;
					const bool adapter_supports_dx12 = SUCCEEDED(::D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr));

					if (adapter_is_software)
					{
						// Don't select the Basic Render Driver adapter.
						// If you want a software adapter, pass in "/warp" on the command line.
						continue;
					}

					// Check to see whether the adapter supports Direct3D 12, but don't create the
					// actual device yet.
					if (adapter_supports_dx12)
					{
						physical_devices.push_back(adapter);
					}
				}
			}
			
			// store the first device we found...
			edited_args.m_physdevice = physical_devices[0];
		}

		// check if physical device is valid
		if (edited_args.m_physdevice == nullptr)
			return result_type::make_error("desc.m_physdevice is nullptr!");

		auto dxphysdevice = cast<dx12_physdevice>(edited_args.m_physdevice.value());
		if (!dxphysdevice) 
			return result_type::make_error("args.m_physdevice failed casting to dx12_physdevice!");

		// enable the debug layer
		if (edited_args.m_debug)
		{
			ID3D12Debug* debugController;
			if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
			{
				debugController->EnableDebugLayer();
			}
			debugController->Release();
		}

		// create the logical device
		dx12_device* dxdevice{};
		hres = ::D3D12CreateDevice(
			dxphysdevice.get(),
			D3D_FEATURE_LEVEL_11_0,
			IID_PPV_ARGS(&dxdevice));

		if (edited_args.m_debug)
		{
			ID3D12InfoQueue* info_queue;
			hres = dxdevice->QueryInterface(IID_PPV_ARGS(&info_queue));
			if (hres == S_OK)
			{
				D3D12_MESSAGE_ID hide[] =
				{
					D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,
	#if 0
						D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,
						// Workarounds for debug layer issues on hybrid-graphics systems
						D3D12_MESSAGE_ID_EXECUTECOMMANDLISTS_WRONGSWAPCHAINBUFFERREFERENCE,
						D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE,
						D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,
						D3D12_MESSAGE_ID_CLEARDEPTHSTENCILVIEW_MISMATCHINGCLEARVALUE
	#endif
				};
				D3D12_INFO_QUEUE_FILTER filter = {};
				filter.DenyList.NumIDs = _countof(hide);
				filter.DenyList.pIDList = hide;
				info_queue->AddStorageFilterEntries(&filter);
			}
		}

		// create internal descriptor heaps
		// (only if we can carry that into out_data)
		dx12_descheap* int_rtv_heap = nullptr;
		dx12_descheap* int_dsv_heap = nullptr;
		if (out_data != nullptr)
		{
			descheap_create_args descheap_args{};
			descheap_args.m_device = dxdevice;
			descheap_args.m_num_descriptors = k_max_num_rendertargets_per_draw;
			descheap_args.m_shader_visible = false;
			descheap_args.m_type = e_descriptor_heap_type::rtv;
			auto rtv_heap = create_native(descheap_args);
			if (!rtv_heap)
				return result_type::make_error("dx12 requires creating an internal RTV heap, but that failed (for some reason)");
			int_rtv_heap = rtv_heap.get();

			descheap_args.m_type = e_descriptor_heap_type::dsv;
			descheap_args.m_num_descriptors = 1u;
			auto dsv_heap = create_native(descheap_args);
			if (!dsv_heap)
				return result_type::make_error("dx12 requires creating an internal DSV heap, but that failed (for some reason)");
			int_dsv_heap = dsv_heap.get();
		}

		// setup output data:
		// - descriptor strides
		// - dxgi factory
		if (out_data != nullptr)
		{
			out_data->m_physical_device = dxphysdevice.get();
			out_data->m_instance = out_data->m_instance = dxfactory;
			out_data->m_descriptor_strides[0] = dxdevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
			out_data->m_descriptor_strides[1] = dxdevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
			out_data->m_descriptor_strides[2] = dxdevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			out_data->m_descriptor_strides[3] = dxdevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
			out_data->m_internal_rtv_heap = int_rtv_heap;
			out_data->m_internal_dsv_heap = int_dsv_heap;
		}
		
		return hres_to_result<native_device>(hres, dxdevice);
	}

	result<native_queue> create_native(const queue_create_args& args, queue_data* out_data)
	{
		using result_type = result<native_queue>;

		if (args.m_device == nullptr)
			return result_type::make_error("args.m_device is nullptr!");

		auto device = cast<dx12_device>(args.m_device);
		if (!device)
			return result_type::make_error("args.m_device failed casting to dx12_device!");

		D3D12_COMMAND_QUEUE_DESC dxdesc{};
		dxdesc.Type = translate(args.m_type);
		dxdesc.Priority = args.m_priority;
		dxdesc.Flags;

		ID3D12CommandQueue* queue{};
		HRESULT hres = device->CreateCommandQueue(&dxdesc, IID_PPV_ARGS(&queue));
		return hres_to_result<native_queue>(hres, queue);
	}

	result<native_swapchain> create_native(const swapchain_create_args& args, swapchain_data* out_data)
	{
		using result_type = result<native_swapchain>;

		if (args.m_device == nullptr)
			return result_type::make_error("args.m_device is nullptr!");

		auto dxfactory = cast<dx12_factory>(args.m_instance);
		if (!dxfactory) return result_type::make_error("device carries no owning factory to create swapchain!");

		auto dxqueue = cast<dx12_queue>(args.m_queue);
		if (!dxqueue) return result_type::make_error("failed casting queue to dx12_queue!");

		if (args.m_queue == nullptr)									return result_type::make_error("args.m_queue is nullptr!");
		if (args.m_dimensions.is_zero())								return result_type::make_error("args.m_dimensions are invalid!");
		if (args.m_num_buffers <= 0u || args.m_num_buffers > 3)			return result_type::make_error("args.m_num_buffers is not valid!");
		if (!swapchain::is_swapchain_format_supported(args.m_format))	return result_type::make_error("args.m_format is not swapchain supported!");

		const uint32 width = args.m_dimensions.x;
		const uint32 height = args.m_dimensions.y;

		// create dx swapchain
		DXGI_SWAP_CHAIN_DESC1 dxdesc = {};
		dxdesc.BufferCount = args.m_num_buffers;
		dxdesc.Width = width;
		dxdesc.Height = height;
		dxdesc.Format = translate(args.m_format);
		dxdesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		dxdesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		dxdesc.SampleDesc.Count = 1;
		dxdesc.Flags;

		IDXGISwapChain1* int_swapchain;
		HRESULT hres = dxfactory->CreateSwapChainForHwnd(
			dxqueue.get(),
			(HWND)args.m_window,
			&dxdesc,
			nullptr,
			nullptr,
			&int_swapchain);
		dx12_swapchain* swapchain = (dx12_swapchain*)int_swapchain;

		auto swapchain_create_res = hres_to_result<object_native>(hres, swapchain);
		if (!swapchain_create_res)
			return result_type::make_error("failed creating swapchain for Hwnd!");

		// associate the swapchain with the passed window
		hres = dxfactory->MakeWindowAssociation((::HWND)args.m_window, DXGI_MWA_NO_ALT_ENTER);
		if (!hres_to_result<native_swapchain>(hres, swapchain))
		{
			return result_type::make_warning(swapchain, "after creating the swapchain, failed making window association! (result is still valid)");
		}

		// if specified, create a descriptor heap with rtvs to backbuffers
		if (out_data != nullptr && args.m_own_descriptors)
		{
			rhi::descheap_create_args heap_desc{};
			heap_desc.m_device = args.m_device;
			heap_desc.m_num_descriptors = args.m_num_buffers;
			heap_desc.m_type = rhi::e_descriptor_heap_type::rtv;
			auto rtv_heap = create_native(heap_desc);
			if (!rtv_heap)
			{
				return result_type::make_error("own_descriptors: failed creating descriptor heap for rtvs");
			}

			// set all rtvs as dirty
			for (uint32 i = 0u; i < args.m_num_buffers; ++i)
				out_data->m_rtv_dirty_list.push_back(true);

			// store an rtv heap
			out_data->m_rtv_heap = (native_descheap)rtv_heap.get();
		}

		return swapchain;
	}

	result<native_descheap> create_native(const descheap_create_args& args, descheap_data* out_data)
	{
		using result_type = result<native_descheap>;
		if (args.m_device == nullptr)
			return result_type::make_error("args.m_device is nullptr!");

		auto device = cast<dx12_device>(args.m_device);
		if (!device)
			return result_type::make_error("args.m_device failed casting to dx12_device!");

		ID3D12DescriptorHeap* descheap{};
		D3D12_DESCRIPTOR_HEAP_DESC dxdesc{};
		dxdesc.Flags;
		dxdesc.NumDescriptors = args.m_num_descriptors;
		dxdesc.Type = translate(args.m_type);
		dxdesc.NodeMask;
		
		HRESULT hres = device->CreateDescriptorHeap(&dxdesc, IID_PPV_ARGS(&descheap));

		if (out_data)
		{
			out_data->m_descriptor_stride = device->GetDescriptorHandleIncrementSize(translate(args.m_type));
			out_data->m_freelist.resize(args.m_num_descriptors, false);
		}

		return hres_to_result<native_descheap>(hres, descheap);
	}

	result<native_commandpool> create_native(const commandpool_create_args& args, commandpool_data* out_data)
	{
		using result_type = result<native_commandpool>;
		if (args.m_device == nullptr)
			return result_type::make_error("args.m_device is nullptr!");

		auto device = cast<dx12_device>(args.m_device);
		if (!device)
			return result_type::make_error("args.m_device failed casting to dx12_device!");
		
		ID3D12CommandAllocator* allocator{};
		HRESULT hres = device->CreateCommandAllocator(translate(args.m_type), IID_PPV_ARGS(&allocator));
		return hres_to_result<native_commandpool>(hres, allocator);
	}

	result<native_commandlist> create_native(const commandlist_create_args& args, commandlist_data* out_data)
	{
		using result_type = result<native_commandlist>;
		
		if (args.m_device == nullptr)
			return result_type::make_error("args.m_device is nullptr!");

		auto dxdevice = cast<dx12_device>(args.m_device);
		if (!dxdevice) return result_type::make_error("args.m_device failed casting to dx12_device!");

		// if allocator is not provided, we need to create our own
		commandlist_create_args edited_args = args;
		if (edited_args.m_pool == nullptr)
		{
			commandpool_create_args pool_args{};
			pool_args.m_type = edited_args.m_type;
			pool_args.m_device = edited_args.m_device;
			auto res = create_native(pool_args);
			if (!res) 
				return result_type::make_error("failed creating allocator!");

			edited_args.m_pool = (native_commandpool)res.get();
		}

		auto dxallocator = cast<dx12_allocator>(edited_args.m_pool.value());
		if (!dxallocator) return result_type::make_error("args.m_allocator failed casting to dx12_allocator!");

		dx12_commandlist* commandlist{};
		HRESULT hres = dxdevice->CreateCommandList(
			0u, 
			translate(edited_args.m_type),
			dxallocator.get(),
			nullptr,
			IID_PPV_ARGS(&commandlist));
		hres = commandlist->Close();

		// store allocator for later use
		if (out_data != nullptr)
		{
			out_data->m_current_pool = edited_args.m_pool.value();
		}

		// create & store a new fence if instructed
		if (args.m_own_fence && out_data != nullptr)
		{
			fence_create_args fence_args{};
			fence_args.m_device = args.m_device;
			fence_args.m_init_value = 0u;
			auto fence = create_native(fence_args);
			if (!fence) return result_type::make_error("create commandlist: failed creating fence!");

			out_data->m_fence = (native_fence)fence.get();
		}

		return hres_to_result<native_commandlist>(hres, commandlist);
	}
	
	result<native_fence> create_native(const fence_create_args& args, fence_data* out_data)
	{
		using result_type = result<native_fence>;
		if (args.m_device == nullptr)
			return result_type::make_error("args.m_device is nullptr!");

		auto device = cast<dx12_device>(args.m_device);
		if (!device)
			return result_type::make_error("args.m_device failed casting to dx12_device!");

		ID3D12Fence* fence{};
		D3D12_FENCE_FLAGS flags{};
		HRESULT hres = device->CreateFence(args.m_init_value, flags, IID_PPV_ARGS(&fence));
		return hres_to_result<native_fence>(hres, fence);
	}

	template <typename e_object _e>
	static result<native_type<_e>> create_native(const create_args<_e>& args, data_type<_e>* out_data)
	{
		static constexpr bool k_is_texture = _e == e_object::texture;

		using result_type = result<native_type<_e>>;
		auto dxdevice = cast<dx12_device>(args.m_device);
		if (!dxdevice)
			return result_type::make_error("args.m_device failed casting to dx12_device!");

		D3D12_RESOURCE_DESC dxdesc{};
		if constexpr (k_is_texture)
		{
			dxdesc.Dimension = translate(e_resource_type::texture, args.m_type);
			dxdesc.Format = translate(args.m_format);
			dxdesc.MipLevels = args.m_num_mips;
			dxdesc.DepthOrArraySize = (args.m_arraysize > 1 ? args.m_arraysize : args.m_dimensions.z);
			dxdesc.Width = args.m_dimensions.x;
			dxdesc.Height = args.m_dimensions.y;
			dxdesc.SampleDesc.Count = 1u;
			dxdesc.SampleDesc.Quality = 0u;
		}
		else
		{
			dxdesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
			dxdesc.Format = DXGI_FORMAT_UNKNOWN;
			dxdesc.MipLevels = 1u;
			dxdesc.DepthOrArraySize = 1u;
			dxdesc.Width = args.m_bytesize;
			dxdesc.Height = 1u;
			dxdesc.SampleDesc.Count = 1u;
			dxdesc.SampleDesc.Quality = 0u;
			dxdesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		}
		
		dxdesc.Alignment = 0u;
		dxdesc.Flags = translate(args.m_bindflags);

		// https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ne-d3d12-d3d12_resource_flags
		if constexpr (!k_is_texture)
			dxdesc.Flags &= ~D3D12_RESOURCE_FLAG_ALLOW_SIMULTANEOUS_ACCESS;
		
		D3D12_CLEAR_VALUE dxclear{};
		const bool allow_optimized_clear = k_is_texture && (dxdesc.Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET) || (dxdesc.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);
		if constexpr (k_is_texture)
		{
			dxclear.Format = translate(args.m_format);
		}
		
		HRESULT hres = {};
		dx12_resource* dxresource = nullptr;
		D3D12_RESOURCE_STATES dxstates = translate(args.m_init_state);

		// VIRTUAL (RESERVED) RESOURCE
		if (args.m_is_virtual)
		{
			// D3D12_RESOURCE_DESC::Layout must be D3D12_TEXTURE_LAYOUT_64KB_UNDEFINED_SWIZZLE when creating reserved resources.
			dxdesc.Layout = D3D12_TEXTURE_LAYOUT_64KB_UNDEFINED_SWIZZLE;
			hres = dxdevice->CreateReservedResource(
				&dxdesc,
				dxstates,
				allow_optimized_clear ? &dxclear : nullptr,
				IID_PPV_ARGS(&dxresource));
		}
		// PLACED RESOURCE
		else if (args.m_memoryheap != nullptr)
		{
			// hres = dxdevice->CreatePlacedResource();
		}
		// COMMITTED RESOURCE
		else
		{
			D3D12_HEAP_PROPERTIES heap_props{};
			heap_props.Type = D3D12_HEAP_TYPE_DEFAULT;
			D3D12_HEAP_FLAGS heap_flags{};
			if (has_flag(args.m_memoryheap_desc.m_flags, e_memoryheap_flags::cpu_writable))
				heap_props.Type = D3D12_HEAP_TYPE_UPLOAD;

			hres = dxdevice->CreateCommittedResource(
				&heap_props,
				heap_flags,
				&dxdesc,
				dxstates,
				allow_optimized_clear ? &dxclear : nullptr,
				IID_PPV_ARGS(&dxresource));
		}

		if (out_data)
		{
			out_data->m_previous_state = out_data->m_current_state = args.m_init_state;
			if constexpr (_e == e_object::texture)
			{

			}
			else
			{
				
			}
		}

		return hres_to_result<native_type<_e>>(hres, dxresource);
	}

	result<native_buffer> create_native(const buffer_create_args& args, buffer_data* out_data)
	{
		return rhi::create_native<e_object::buffer>(args, out_data);
	}

	result<native_texture> create_native(const texture_create_args& args, texture_data* out_data)
	{
		return rhi::create_native<e_object::texture>(args, out_data);
	}

	result<native_pipeline> create_native(const pipeline_create_args& args, pipeline_data* out_data)
	{
		using result_type = result<native_pipeline>;
		if (!args.is_valid())
			return result_type::make_error("pipeline_create_args are invalid!");

		auto rootsignature = cast<dx12_rootsignature>(args.m_rootsignature);
		if (!rootsignature)
			return result_type::make_error("args.m_rootsignature failed casting to dx12_rootsignature!");

		auto dxdevice = cast<dx12_device>(args.m_device);
		if (!dxdevice)
			return result_type::make_error("args.m_device failed casting to dx12_device!");

		// helper
		const auto get_shader_code = [](const vector<byte>& data) {
				return CD3DX12_SHADER_BYTECODE(data.data(), data.size());
		};

		HRESULT hres{};
		ID3D12PipelineState* dxpipeline = nullptr;
		switch (args.m_type)
		{
		case e_pipeline_type::graphics:
		{
			// input layout
			D3D12_INPUT_LAYOUT_DESC input_layout_desc{};
			vector< D3D12_INPUT_ELEMENT_DESC> input_elements{};
			for (const graphics_pipeline_desc::input_element& element : args.m_graphics.m_input_elements)
			{
				input_elements.push_back({});
				input_elements.back().AlignedByteOffset = element.m_aligned_byteoffset;
				input_elements.back().Format = translate(element.m_format);
				input_elements.back().InputSlot = element.m_input_slot;
				input_elements.back().InputSlotClass = element.m_is_per_instance ? D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA : D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
				input_elements.back().InstanceDataStepRate = element.m_instance_data_steprate;
				input_elements.back().SemanticIndex = element.m_semantic_idx;
				input_elements.back().SemanticName = element.m_semantic_name.c_str();
			}
			input_layout_desc.pInputElementDescs = input_elements.data();
			input_layout_desc.NumElements = (uint32)input_elements.size();

			D3D12_GRAPHICS_PIPELINE_STATE_DESC pso_desc = {};
			pso_desc.InputLayout = input_layout_desc;
			pso_desc.pRootSignature = rootsignature.get();
			pso_desc.VS = get_shader_code(args.m_graphics_shaders.get(e_graphics_shader_slots::vs));
			pso_desc.PS = get_shader_code(args.m_graphics_shaders.get(e_graphics_shader_slots::ps));
			pso_desc.DS = get_shader_code(args.m_graphics_shaders.get(e_graphics_shader_slots::ds));
			pso_desc.GS = get_shader_code(args.m_graphics_shaders.get(e_graphics_shader_slots::gs));
			pso_desc.HS = get_shader_code(args.m_graphics_shaders.get(e_graphics_shader_slots::hs));
			pso_desc.RasterizerState = translate(args.m_graphics.m_rasterizer);
			pso_desc.DepthStencilState = translate(args.m_graphics.m_output_merger.m_depthtarget);
			pso_desc.SampleMask = args.m_graphics.m_sample_desc.m_sample_mask;
			pso_desc.PrimitiveTopologyType = translate_type(args.m_graphics.m_primitive_topology);
			pso_desc.DSVFormat = translate(args.m_graphics.m_output_merger.m_depthtarget.m_format);
			pso_desc.SampleDesc.Count = args.m_graphics.m_sample_desc.m_num_samples;
			pso_desc.SampleDesc.Quality = args.m_graphics.m_sample_desc.m_quality;
			pso_desc.BlendState.AlphaToCoverageEnable = args.m_graphics.m_blend_alpha_to_coverage_enabled;
			pso_desc.BlendState.IndependentBlendEnable = args.m_graphics.m_blend_independent;
			for (size_t i = 0u; i < k_max_num_rendertargets_per_draw; ++i)
			{
				const auto& colour_target = args.m_graphics.m_output_merger.m_rendertargets[i];
				if (colour_target.m_enabled == false) continue;

				pso_desc.NumRenderTargets++;
				pso_desc.RTVFormats[i] = translate(colour_target.m_format);
				pso_desc.BlendState.RenderTarget[i].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
				pso_desc.BlendState.RenderTarget[i].BlendEnable		= colour_target.m_blend.m_enabled;
				pso_desc.BlendState.RenderTarget[i].BlendOp			= translate(colour_target.m_blend.m_op);
				pso_desc.BlendState.RenderTarget[i].BlendOpAlpha	= translate(colour_target.m_blend.m_op_alpha);
				pso_desc.BlendState.RenderTarget[i].DestBlend		= translate(colour_target.m_blend.m_dest);
				pso_desc.BlendState.RenderTarget[i].DestBlendAlpha	= translate(colour_target.m_blend.m_destalpha);
				pso_desc.BlendState.RenderTarget[i].LogicOp			= translate(colour_target.m_blend.m_logic_op);
				pso_desc.BlendState.RenderTarget[i].LogicOpEnable	= colour_target.m_blend.m_logic_enabled;
				pso_desc.BlendState.RenderTarget[i].SrcBlend		= translate(colour_target.m_blend.m_src);
				pso_desc.BlendState.RenderTarget[i].SrcBlendAlpha	= translate(colour_target.m_blend.m_srcalpha);
			}

			HRESULT hres = dxdevice->CreateGraphicsPipelineState(&pso_desc, IID_PPV_ARGS(&dxpipeline));
			if (hres != S_OK)
				return result_type::make_error("CreateGraphicsPipelineState failed!");
		}
		break;

		case e_pipeline_type::compute:
		{
			D3D12_COMPUTE_PIPELINE_STATE_DESC desc{};
			desc.CachedPSO;
			desc.CS = get_shader_code(args.m_compute_shaders.get(e_compute_shader_slots::cs));
			desc.Flags;
			desc.NodeMask;
			desc.pRootSignature = rootsignature.get();
			hres = dxdevice->CreateComputePipelineState(&desc, IID_PPV_ARGS(&dxpipeline));
		}
		break;
		case e_pipeline_type::raytracing:
		{

		}
		break;

		default:
			return result_type::make_error("args.m_type is unsupported!");
		}

		// fill out data
		if (out_data)
		{
			// ...
		}

		return dxpipeline;
	}

	result<native_rootsignature> create_native(const rootsignature_create_args& args, rootsignature_data* out_data)
	{
		using result_type = result<native_rootsignature>;

		auto device = cast<dx12_device>(args.m_device);
		if (!device)
			return result_type::make_error("args.m_device failed casting to dx12_device!");
		dx12_device* dxdevice = device.get();

		ID3D12RootSignature* dxrootsignature = nullptr;

		// setup versioning
		D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
		// This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
		if (dxdevice->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData)) != S_OK)
		{
			featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
		}

		// build a name-to-param-idx map
		umap<string, uint32> name_to_param_idx{};

		// setup root parameters
		vector<CD3DX12_ROOT_PARAMETER1> root_parameters{};
		vector<CD3DX12_STATIC_SAMPLER_DESC> static_samplers{};
		vector<vector<CD3DX12_DESCRIPTOR_RANGE1>> root_descriptor_ranges(args.m_resource_tables.size());

		// constants
		for (const root_param_constants& constants : args.m_constants)
		{
			name_to_param_idx[constants.m_common.m_name] = (uint32)root_parameters.size();

			root_parameters.push_back({});
			root_parameters.back().InitAsConstants(constants.m_num_dwords, constants.m_common.m_shader_register,
				constants.m_common.m_register_space, translate(constants.m_common.m_visibility));
		}

		// resources
		for (const root_param_resource& resource : args.m_resources)
		{
			name_to_param_idx[resource.m_common.m_name] = (uint32)root_parameters.size();

			root_parameters.push_back({});
			switch (resource.m_type)
			{
			case root_param_resource::e_type::srv:
				root_parameters.back().InitAsShaderResourceView(
					resource.m_common.m_shader_register,
					resource.m_common.m_register_space,
					D3D12_ROOT_DESCRIPTOR_FLAG_NONE,
					translate(resource.m_common.m_visibility));
				break;

			case root_param_resource::e_type::cbv:
				root_parameters.back().InitAsConstantBufferView(
					resource.m_common.m_shader_register,
					resource.m_common.m_register_space,
					D3D12_ROOT_DESCRIPTOR_FLAG_NONE,
					translate(resource.m_common.m_visibility));
				break;

			case root_param_resource::e_type::uav:
				root_parameters.back().InitAsUnorderedAccessView(
					resource.m_common.m_shader_register,
					resource.m_common.m_register_space,
					D3D12_ROOT_DESCRIPTOR_FLAG_NONE,
					translate(resource.m_common.m_visibility));
				break;
			}
		}

		// resource tables
		size_t descriptor_table_idx = 0u;
		for (const root_param_resource_table& table : args.m_resource_tables)
		{
			name_to_param_idx[table.m_common.m_name] = (uint32)root_parameters.size();

			vector<CD3DX12_DESCRIPTOR_RANGE1>& ranges = root_descriptor_ranges[descriptor_table_idx++];
			for (const root_param_resource_range& range : table.m_resource_ranges)
			{
				D3D12_DESCRIPTOR_RANGE_TYPE range_type{};
				switch (range.m_type)
				{
				case root_param_resource_range::e_type::cbv: range_type = D3D12_DESCRIPTOR_RANGE_TYPE_CBV; break;
				case root_param_resource_range::e_type::uav: range_type = D3D12_DESCRIPTOR_RANGE_TYPE_UAV; break;
				case root_param_resource_range::e_type::srv: range_type = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; break;
				case root_param_resource_range::e_type::sampler: range_type = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER; break;
				}

				ranges.push_back({});
				ranges.back().Init(range_type,
					range.m_num_resources,
					range.m_shader_register,
					range.m_register_space);
			}

			root_parameters.push_back({});
			root_parameters.back().InitAsDescriptorTable((uint32)ranges.size(), ranges.data(), translate(table.m_common.m_visibility));
		}

		// samplers
		for (const root_static_sampler& sampler : args.m_static_samplers)
		{
			static_samplers.push_back({});
			static_samplers.back().Init(
				sampler.m_common.m_shader_register,
				translate(sampler.m_filter),
				translate(sampler.m_wrap_u),
				translate(sampler.m_wrap_v),
				translate(sampler.m_wrap_w),
				sampler.m_mip_lod_bias,
				sampler.m_max_anisotropy,
				translate(sampler.m_comparison_func),
				translate(sampler.m_border_color),
				sampler.m_min_lod,
				sampler.m_max_lod,
				translate(sampler.m_common.m_visibility));
		}

		// initialize the desc, and create the root signature
		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.Init_1_1(
			(uint32)root_parameters.size(), root_parameters.data(),
			(uint32)static_samplers.size(), static_samplers.data(),
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		// flag direct indexing enabled/disabled
		if (args.m_bindless)
		{
			rootSignatureDesc.Desc_1_1.Flags |= D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED;
			rootSignatureDesc.Desc_1_1.Flags |= D3D12_ROOT_SIGNATURE_FLAG_SAMPLER_HEAP_DIRECTLY_INDEXED;
		}

		ID3DBlob* signature;
		ID3DBlob* error;

		HRESULT
		res = D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error);
		if (res != S_OK && error != nullptr)
		{
			return result_type::make_error(static_cast<const char*>(error->GetBufferPointer()));
		}
		res = dxdevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&dxrootsignature));

		return dxrootsignature;
	}

	result<native_memoryheap> create_native(const memheap_create_args& args, memheap_data* out_data)
	{
		using result_type = result<native_memoryheap>;
		
		if (args.m_bytesize <= 0u)
			return result_type::make_error("args.m_bytesize == 0u, nothing happened...");

		auto device = cast<dx12_device>(args.m_device);
		if (!device)
			return result_type::make_error("args.m_device failed casting to dx12_device!");

		D3D12_HEAP_DESC heapDesc = {};
		heapDesc.SizeInBytes = args.m_bytesize;
		// heapDesc.SizeInBytes = tileCount * D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
		heapDesc.Properties.Type = D3D12_HEAP_TYPE_DEFAULT;
		// heapDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
		heapDesc.Flags = D3D12_HEAP_FLAG_NONE;

		ID3D12Heap* heap;
		auto hres = device->CreateHeap(&heapDesc, IID_PPV_ARGS(&heap));
		if (hres != S_OK)
			return result_type::make_error("ID3D12Device::CreateHeap failed!");

		if (out_data)
		{

		}
		return heap;
	}

	result<> release(object_native native)
	{
		using result_type = result<>;
		auto as_unknown = cast<IUnknown>(native);
		if (!as_unknown) return result_type::make_error("failed casting native to IUnknown");

		ULONG hres = as_unknown.get()->Release();
		if (!hres)
			return result_type::make_error("failed releasing native object!");
		
		return {};
	}

	result<buffer> import_buffer(native_buffer native)
	{
		using result_type = result<buffer>;

		auto dxresource = cast<dx12_resource>(native);
		if (!dxresource)
			return result_type::make_error("native failed casting to dx12_resource!");

		D3D12_RESOURCE_DESC desc = dxresource->GetDesc();

		buffer imported{};
		imported.m_create_args.m_bytesize = desc.Width;
		imported.m_create_args.m_bytestride = 1u;
		imported.m_native_object = native;
		imported.m_create_args.m_device = nullptr;
		return imported;
	}

	result<texture> import_texture(native_texture native)
	{
		using result_type = result<texture>;

		auto dxresource = cast<dx12_resource>(native);
		if (!dxresource)
			return result_type::make_error("native failed casting to dx12_resource!");

		D3D12_RESOURCE_DESC desc = dxresource->GetDesc();
		
		dx12_device* dxdevice = nullptr;
		HRESULT hres = dxresource->GetDevice(IID_PPV_ARGS(&dxdevice));
		if (dxdevice == nullptr || hres != S_OK)
			return result_type::make_error("couldn't fetch owner device from existing resource");

		texture imported{};
		imported.m_native_object = native;
		imported.m_create_args.m_device = dxdevice;
		imported.m_create_args = texture_create_args::tex2D({ desc.Width, desc.Height });
		imported.m_create_args;
		return imported;
	}

	result<descheap> import_descheap(native_descheap native)
	{
		using result_type = result<descheap>;

		auto dxheap = cast<dx12_descheap>(native);
		if (!dxheap)
			return result_type::make_error("native failed casting to dx12_descheap!");

		dx12_device* dxdevice = nullptr;
		HRESULT res = dxheap->GetDevice(IID_PPV_ARGS(&dxdevice));
		if (dxdevice == nullptr)
			return result_type::make_error("couldn't fetch owner device from existing heap");

		auto dxdesc = dxheap->GetDesc();		
		const D3D12_DESCRIPTOR_HEAP_TYPE dxtype = dxdesc.Type;
		const uint32 num_descriptors = dxdesc.NumDescriptors;

		auto query = query_descriptor_stride(dxdevice, dxtype);
		if (!query)
			return result_type::make_error("failed query_descriptor_stride!");

		descheap imported{};
		imported.m_data.m_descriptor_stride = query.get();
		imported.m_create_args.m_device = dxdevice;
		imported.m_create_args.m_num_descriptors = num_descriptors;
		imported.m_create_args.m_type = translate(dxtype);
		imported.m_native_object = native;
		imported.m_data.m_freelist.resize(num_descriptors, true);
		return imported;
	}
	
	result<commandlist> import_commandlist(native_commandlist native)
	{
		using result_type = result<commandlist>;

		auto dxcommandlist = cast<dx12_commandlist>(native);
		if (!dxcommandlist)
			return result_type::make_error("native failed casting to dx12_commandlist!");

		dx12_device* dxdevice = nullptr;
		HRESULT res = dxcommandlist->GetDevice(IID_PPV_ARGS(&dxdevice));
		if (dxdevice == nullptr)
			return result_type::make_error("couldn't fetch owner device from existing");

		commandlist imported{};
		imported.m_create_args.m_pool;
		imported.m_create_args.m_device;
		imported.m_create_args.m_type;
		imported.m_native_object = native;
		imported.m_data.m_current_pool;
		return imported;
	}
	
	result<commandpool> import_commandpool(native_commandpool native)
	{
		using result_type = result<commandpool>;

		auto dxallocator = cast<dx12_allocator>(native);
		if (!dxallocator)
			return result_type::make_error("native failed casting to dx12_allocator!");

		dx12_device* dxdevice = nullptr;
		HRESULT res = dxallocator->GetDevice(IID_PPV_ARGS(&dxdevice));
		if (dxdevice == nullptr)
			return result_type::make_error("couldn't fetch owner device from existing");

		commandpool imported{};
		imported.m_create_args.m_device = dxdevice;
		imported.m_create_args.m_type;
		imported.m_native_object = native;
		return imported;
	}

	result<> device::create_rtv(const texture& texture, descriptor descriptor)
	{
		using result_type = result<>;

		auto device = cast<dx12_device>(m_native_object);
		if (!device)
			return result_type::make_error("m_native_object failed casting to dx12_device!");

		if (descriptor.m_cpu_address == 0u)
			return result_type::make_error("descriptor is nullptr!");

		auto resource = cast<dx12_resource>(texture.m_native_object);
		if (!resource)
			return result_type::make_error("texture.m_native_object failed casting to dx12_resource!");

		const auto format = texture.get_format();
		if (!is_format_supported(format))
			return result_type::make_error("texture has an unsupported format!");

		D3D12_RENDER_TARGET_VIEW_DESC desc{};
		desc.Texture2D.MipSlice;
		desc.Texture2D.PlaneSlice;
		desc.Format = translate(format);
		desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

		D3D12_CPU_DESCRIPTOR_HANDLE dxdescriptor{ .ptr = descriptor.m_cpu_address };
		device->CreateRenderTargetView(resource.get(), &desc, dxdescriptor);
		return {};
	}
	
	result<> device::create_dsv(const texture& texture, descriptor descriptor)
	{
		using result_type = result<>;

		auto device = cast<dx12_device>(m_native_object);
		if (!device)
			return result_type::make_error("m_native_object failed casting to dx12_device!");

		auto resource = cast<dx12_resource>(texture.m_native_object);
		if (!resource)
			return result_type::make_error("texture.m_native_object failed casting to dx12_resource!");

		const auto format = texture.get_format();
		if (!is_format_supported(format))
			return result_type::make_error("texture has an unsupported format!");

		D3D12_DEPTH_STENCIL_VIEW_DESC desc{};
		desc.Texture2D.MipSlice;
		desc.Format = translate(format);
		desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;

		D3D12_CPU_DESCRIPTOR_HANDLE dxdescriptor{ .ptr = descriptor.m_cpu_address };
		device->CreateDepthStencilView(resource.get(), &desc, dxdescriptor);
		return {};
	}
	result<> device::create_sampview(const sampler& sampler, descriptor descriptor)
	{
		return {};
	}
	result<> device::create_srv(const texture& texture, descriptor descriptor)
	{
		using result_type = result<>;

		auto device = cast<dx12_device>(m_native_object);
		if (!device)
			return result_type::make_error("m_native_object failed casting to dx12_device!");

		auto resource = cast<dx12_resource>(texture.m_native_object);
		if (!resource)
			return result_type::make_error("texture.m_native_object failed casting to dx12_resource!");

		const auto format = texture.get_format();
		if (!is_format_supported(format))
			return result_type::make_error("texture has an unsupported format!");

		D3D12_SHADER_RESOURCE_VIEW_DESC desc{};
		desc.Texture2D.MipLevels;
		desc.Texture2D.MostDetailedMip;
		desc.Texture2D.PlaneSlice;
		desc.Texture2D.ResourceMinLODClamp;
		desc.Format = translate(format);
		desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;

		D3D12_CPU_DESCRIPTOR_HANDLE dxdescriptor{ .ptr = descriptor.m_cpu_address };
		device->CreateShaderResourceView(resource.get(), &desc, dxdescriptor);
		return {};
	}
	result<> device::create_uav(const texture& texture, descriptor descriptor)
	{
		using result_type = result<>;

		auto device = cast<dx12_device>(m_native_object);
		if (!device)
			return result_type::make_error("m_native_object failed casting to dx12_device!");

		auto resource = cast<dx12_resource>(texture.m_native_object);
		if (!resource)
			return result_type::make_error("texture.m_native_object failed casting to dx12_resource!");

		const auto format = texture.get_format();
		if (!is_format_supported(format))
			return result_type::make_error("texture has an unsupported format!");

		D3D12_UNORDERED_ACCESS_VIEW_DESC desc{};
		desc.Texture2D.MipSlice;
		desc.Texture2D.PlaneSlice;
		desc.Format = translate(format);
		desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;

		D3D12_CPU_DESCRIPTOR_HANDLE dxdescriptor{ .ptr = descriptor.m_cpu_address };
		device->CreateUnorderedAccessView(resource.get(), nullptr, &desc, dxdescriptor);
		return {};
	}
	result<> device::create_srv(const buffer& buffer, descriptor descriptor)
	{
		using result_type = result<>;

		auto device = cast<dx12_device>(m_native_object);
		if (!device)
			return result_type::make_error("m_native_object failed casting to dx12_device!");

		auto resource = cast<dx12_resource>(buffer.m_native_object);
		if (!resource)
			return result_type::make_error("buffer.m_native_object failed casting to dx12_resource!");

		D3D12_SHADER_RESOURCE_VIEW_DESC desc{};
		desc.Buffer.FirstElement = 0u;
		desc.Buffer.Flags;
		desc.Buffer.NumElements = (uint32)buffer.get_num_elements();
		desc.Buffer.StructureByteStride = (uint32)buffer.get_bytestride();
		desc.Format = DXGI_FORMAT_UNKNOWN;
		desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;

		D3D12_CPU_DESCRIPTOR_HANDLE dxdescriptor{ .ptr = descriptor.m_cpu_address };
		device->CreateShaderResourceView(resource.get(), &desc, dxdescriptor);
		return {};
	}
	result<> device::create_uav(const buffer& buffer, descriptor descriptor)
	{
		using result_type = result<>;

		auto device = cast<dx12_device>(m_native_object);
		if (!device)
			return result_type::make_error("m_native_object failed casting to dx12_device!");

		auto resource = cast<dx12_resource>(buffer.m_native_object);
		if (!resource)
			return result_type::make_error("buffer.m_native_object failed casting to dx12_resource!");

		D3D12_UNORDERED_ACCESS_VIEW_DESC desc{};
		desc.Buffer.FirstElement = 0u;
		desc.Buffer.CounterOffsetInBytes;
		desc.Buffer.Flags;
		desc.Buffer.NumElements = (uint32)buffer.get_num_elements();
		desc.Buffer.StructureByteStride = (uint32)buffer.get_bytestride();
		desc.Format = DXGI_FORMAT_UNKNOWN;
		desc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;

		D3D12_CPU_DESCRIPTOR_HANDLE dxdescriptor{ .ptr = descriptor.m_cpu_address };
		device->CreateUnorderedAccessView(resource.get(), nullptr, &desc, dxdescriptor);
		return {};
	}

	template <typename _t>
	static result<resource_tileinfo> get_tiling_info(const _t& resource, const device& device);

	// [buffer]
	result<void*> buffer::map_begin(const map_args& args)
	{
		using result_type = result<void*>;

		auto resource = cast<dx12_resource>(m_native_object);
		if (!resource)
			return result_type::make_error("buffer.m_native_object failed casting to dx12_resource!");

		void* result;
		D3D12_RANGE range{};
		range.Begin = args.m_offset;
		range.End = math::minimum<uint64>((uint64)(args.m_bytesize - args.m_offset), m_create_args.m_bytesize);
		HRESULT hres = resource->Map(0u, &range, &result);
		if (hres != S_OK)
			return result_type::make_error("ID3D12Resource::Map() failed");
		return result;
	}
	result<> buffer::map_end(const map_args& args)
	{
		using result_type = result<>;

		auto resource = cast<dx12_resource>(m_native_object);
		if (!resource)
			return result_type::make_error("buffer.m_native_object failed casting to dx12_resource!");

		D3D12_RANGE range{};
		range.Begin = args.m_offset;
		range.End = math::minimum<uint64>((uint64)(args.m_bytesize - args.m_offset), m_create_args.m_bytesize);
		resource->Unmap(0u, &range);
		return {};
	}
	result<resource_tileinfo> buffer::get_tiling_info(const device& device) const
	{
		return rhi::get_tiling_info<rhi::buffer>(*this, device);
	}
	
	// [fence - interface]
	result<> fence::queue_signal(uint64 signal_value, const queue& queue)
	{
		return queue.queue_signal(*this, signal_value);
	}

	// [queue - interface]
	result<> queue::submit(vector<commandlist*> commandlists) const
	{
		using result_type = result<>;

		if (m_native_object == nullptr)
			return result_type::make_error("m_native_object is nullptr!");

		auto dxqueue = cast<dx12_queue>(m_native_object);
		if (!dxqueue)
			return result_type::make_error("m_native_object failed casting to dx12_queue!");

		vector<ID3D12CommandList*> dxcommandlists{};
		for (commandlist* list : commandlists)
		{
			if (list->is_recording()) list->end();
			dxcommandlists.push_back((ID3D12CommandList*)list->m_native_object);
		}

		dxqueue->ExecuteCommandLists((uint32)dxcommandlists.size(), dxcommandlists.data());

		// signal finish
		for (commandlist* list : commandlists)
		{
			if (list->has_fence())
			{
				uint32& complete_value = list->m_data.m_complete_value;
				complete_value += 1u;
				queue_signal(list->m_data.m_fence, complete_value);
			}
		}

		return {};
	}
	result<> queue::queue_signal(const fence& fence, uint64 signal_value) const
	{
		return queue_signal(fence.m_native_object, signal_value);
	}
	result<> queue::queue_signal(native_fence fence, uint64 signal_value) const
	{
		using result_type = result<>;

		if (m_native_object == nullptr) 
			return result_type::make_error("m_native_object is nullptr!");

		auto dxqueue = cast<dx12_queue>(m_native_object);
		if (!dxqueue) 
			return result_type::make_error("m_native_object failed casting to dx12_queue!");

		if (fence == nullptr)
			return result_type::make_error("fence is nullptr!");

		auto dxfence = cast<dx12_fence>(fence);
		if (!dxqueue) return result_type::make_error("fence failed casting to dx12_fence!");

		HRESULT hres = dxqueue->Signal(dxfence.get(), signal_value);
		return hres_to_result<>(hres, {});
	}
	
	template <typename _t>
	static result<vmemory_map_result> map_vmemory(
		const queue& queue,
		const device& device,
		const _t& resource,
		memheap& heap,
		const vmemory_map_args& args)
	{
		using result_type = result<vmemory_map_result>;
		if (queue.m_native_object == nullptr)
			return result_type::make_error("queue.m_native_object is nullptr!");

		if (resource.is_virtual() == false)
			return result_type::make_error("resource is not a virtual resource!");

		auto dxqueue = cast<dx12_queue>(queue.m_native_object);
		if (!dxqueue)
			return result_type::make_error("queue.m_native_object failed casting to dx12_queue!");

		auto dxresource = cast<dx12_resource>(resource.m_native_object);
		if (!dxresource)
			return result_type::make_error("texture.m_native_object failed casting to dx12_resource!");

		auto dxheap = cast<dx12_memheap>(heap.m_native_object);
		if (!dxheap)
			return result_type::make_error("heap.m_native_object failed casting to dx12_memheap!");

		auto tiling_info = resource.get_tiling_info(device);
		if (!tiling_info)
			return result_type::make_error("texture.get_tiling_info() failed!");

		auto dxdevice = cast<dx12_device>(device.m_native_object);
		if (!dxdevice)
			return result_type::make_error("device.m_native_object failed casting to dx12_device!");

		D3D12_RESOURCE_DESC desc = dxresource->GetDesc();
		D3D12_RESOURCE_ALLOCATION_INFO allocInfo = dxdevice->GetResourceAllocationInfo(
			0,                  // visible mask
			1,                  // number of resources
			&desc               // resource desc
		);
		const uint64 total_bytes_in_resource = allocInfo.SizeInBytes;
		const uint32 tile_bytesize = (uint32)(total_bytes_in_resource / tiling_info.get().m_num_tiles_total);

		const uint32 num_regions = 1u;
		const uint32 subresource_index = 0u;
		const uint64 num_texels_total = (uint64)args.m_texelrange_size.x
			* (uint64)args.m_texelrange_size.y
			* (uint64)args.m_texelrange_size.z;

		const math::uint3& texels_per_tile = tiling_info.get().m_texels_per_tile;
		const math::uint3 num_tiles = args.m_texelrange_size / texels_per_tile;
		const math::uint3 tilesize_max = tiling_info.get().m_subresource_tilings[subresource_index].m_dimension_in_tiles;
		const math::uint3 clamped_texel_start =
		{
			args.m_texelrange_start.x,
			args.m_texelrange_start.y,
			args.m_texelrange_start.z
		};
		const math::uint3 clamped_tile_range = clamped_texel_start / texels_per_tile;
		const math::uint3 clamped_num_tiles =
		{
			math::minimum(num_tiles.x, tilesize_max.x - clamped_tile_range.x),
			math::minimum(num_tiles.x, tilesize_max.y - clamped_tile_range.y),
			math::minimum(num_tiles.x, tilesize_max.z - clamped_tile_range.z)
		};
		const uint32 num_tiles_needed = clamped_num_tiles.x * clamped_num_tiles.y * clamped_num_tiles.z;;
		uint32 num_tiles_to_map = num_tiles_needed;
		const uint32 num_tiles_in_heap = (uint32)(heap.m_create_args.m_bytesize / tile_bytesize);

		result_type result{};
		if (num_tiles_to_map > num_tiles_in_heap)
		{
			result = result_type::make_warning({}, "heap is not big enough to provide the number of tiles requested by this mapping! This is fine, since we clamp the range, but your virtual memory is not fully mapped!");
			num_tiles_to_map = num_tiles_in_heap;
		}

		vector<D3D12_TILED_RESOURCE_COORDINATE> startCoordinates = {};
		vector<D3D12_TILE_REGION_SIZE> regionSizes = {};
		startCoordinates.resize(num_regions);
		regionSizes.resize(num_regions);
		for (uint32 i = 0u; i < num_regions; ++i)
		{
			startCoordinates[i].X = clamped_texel_start.x;
			startCoordinates[i].Y = clamped_texel_start.y;
			startCoordinates[i].Z = clamped_texel_start.z;
			startCoordinates[i].Subresource = subresource_index;

			// regionSizes[i].Width = clamped_num_tiles.x;
			// regionSizes[i].Height = clamped_num_tiles.y;
			// regionSizes[i].Depth = clamped_num_tiles.z;
			regionSizes[i].NumTiles = num_tiles_to_map;
			regionSizes[i].UseBox = false;
		}

		const uint32 num_heap_ranges = 1u;
		vector<uint32> heap_offsets_in_tiles{};
		vector<uint32> heap_sizes_in_tiles{};
		vector<D3D12_TILE_RANGE_FLAGS> range_flags{};
		heap_offsets_in_tiles.resize(num_heap_ranges);
		heap_sizes_in_tiles.resize(num_heap_ranges);
		range_flags.resize(num_heap_ranges);
		for (uint32 i = 0u; i < num_heap_ranges; ++i)
		{
			heap_offsets_in_tiles[i] = args.m_heap_start;
			range_flags[i] = {};
			heap_sizes_in_tiles[i] = num_tiles_to_map;
		}

		dxqueue->UpdateTileMappings(
			dxresource.get(),
			num_regions,
			startCoordinates.data(),
			regionSizes.data(),
			dxheap.get(),
			num_heap_ranges,
			range_flags.data(),
			heap_offsets_in_tiles.data(),
			heap_sizes_in_tiles.data(),
			D3D12_TILE_MAPPING_FLAG_NONE
		);

		result.get_safe().m_num_tiles_mapped = num_tiles_to_map;
		result.get_safe().m_num_tiles_requested = num_tiles_needed;
		return result;
	}
	
	template <typename _t>
	static result<> unmap_vmemory(
		const queue& queue,
		const device& device,
		const _t& resource,
		const vmemory_unmap_args& args)
	{
		using result_type = result<>;
		auto dxqueue = cast<dx12_queue>(queue.m_native_object);
		if (!dxqueue)
			return result_type::make_error("queue.m_native_object failed casting to dx12_queue!");

		if (resource.is_virtual() == false)
			return result_type::make_error("resource is not a virtual resource!");

		auto dxresource = cast<dx12_resource>(resource.m_native_object);
		if (!dxresource)
			return result_type::make_error("texture.m_native_object failed casting to dx12_resource!");

		auto tiling_info = resource.get_tiling_info(device);
		if (!tiling_info)
			return result_type::make_error("texture.get_tiling_info() failed!");

		const uint32 num_regions = 1u;
		const uint32 subresource_index = 0u;
		const math::uint3& texels_per_tile = tiling_info.get().m_texels_per_tile;
		const uint64 num_texels_total = (uint64)args.m_texelrange_size.x * (uint64)args.m_texelrange_size.y * (uint64)args.m_texelrange_size.z;
		const math::uint3 num_tiles = args.m_texelrange_size / texels_per_tile;
		const math::uint3 max_num_tiles = tiling_info.get().m_subresource_tilings[subresource_index].m_dimension_in_tiles;
		const math::uint3 clamped_texel_start = // todo... we should ACTUALLY clamp this ;)
		{
			args.m_texelrange_start.x,
			args.m_texelrange_start.y,
			args.m_texelrange_start.z
		};
		const math::uint3 clamped_tile_start = clamped_texel_start / texels_per_tile;
		const math::uint3 clamped_num_tiles =
		{
			math::minimum(num_tiles.x, max_num_tiles.x - clamped_tile_start.x),
			math::minimum(num_tiles.x, max_num_tiles.y - clamped_tile_start.y),
			math::minimum(num_tiles.x, max_num_tiles.z - clamped_tile_start.z)
		};
		const uint32 num_tiles_to_unmap = clamped_num_tiles.x * clamped_num_tiles.y * clamped_num_tiles.z;

		vector<D3D12_TILED_RESOURCE_COORDINATE> startCoordinates = {};
		vector<D3D12_TILE_REGION_SIZE> regionSizes = {};
		startCoordinates.resize(num_regions);
		regionSizes.resize(num_regions);
		for (uint32 i = 0u; i < num_regions; ++i)
		{
			startCoordinates[i].X = clamped_texel_start.x;
			startCoordinates[i].Y = clamped_texel_start.y;
			startCoordinates[i].Z = clamped_texel_start.z;
			startCoordinates[i].Subresource = subresource_index;
			regionSizes[i].NumTiles = num_tiles_to_unmap;
			regionSizes[i].UseBox = false;
		}

		vector<D3D12_TILE_RANGE_FLAGS> rangeFlags(1u, D3D12_TILE_RANGE_FLAG_NULL);
		vector<UINT> rangeTileCounts(num_regions, num_tiles_to_unmap);
		vector<UINT> rangeStartOffsets(num_regions, 0); // Not used with NULL flag, but still required

		dxqueue->UpdateTileMappings(
			dxresource.get(),
			num_regions,
			startCoordinates.data(),
			regionSizes.data(),
			nullptr,
			1u,
			rangeFlags.data(),
			rangeStartOffsets.data(),
			rangeTileCounts.data(),
			D3D12_TILE_MAPPING_FLAG_NONE);

		return {};
	}

	result<vmemory_map_result> queue::map_vmemory(const device& device, const texture& texture, memheap& heap, const vmemory_map_args& args) const
	{
		return rhi::map_vmemory<rhi::texture>(*this, device, texture, heap, args);
	}
	result<> queue::unmap_vmemory(const device& device, const texture& texture, const vmemory_unmap_args& args) const
	{
		return rhi::unmap_vmemory<rhi::texture>(*this, device, texture, args);
	}
	result<vmemory_map_result> queue::map_vmemory(const device& device, const buffer& buffer, memheap& heap, const vmemory_map_args& args) const
	{
		return rhi::map_vmemory<rhi::buffer>(*this, device, buffer, heap, args);
	}
	result<> queue::unmap_vmemory(const device& device, const buffer& buffer, const vmemory_unmap_args& args) const
	{
		return rhi::unmap_vmemory<rhi::buffer>(*this, device, buffer, args);
	}

	// [texture]
	result<uint64> texture::calculate_bytesize() const
	{
		using result_type = result<uint64>;
		return get_format().get_bytes_per_pixel() * get_num_pixels();
	}
	result<uint64> texture::calculate_bytestride() const
	{
		return get_format().get_bytes_per_pixel();
	}
	result<> texture::set_name(const char* name)
	{
		using result_type = result<>;

		if (m_native_object == nullptr)
			return result_type::make_error("m_native_object is nullptr!");
		auto dxresource = cast<dx12_resource>(m_native_object);
		if (!dxresource)
			return result_type::make_error("m_native_object failed casting to dx12_resource!");

		wstring wname = to_wstring(name);
		HRESULT hres = dxresource->SetName(wname.c_wstr());
		if (hres != S_OK)
			return result_type::make_error("ID3D12Resource::SetName() failed!");

		return {};
	}

	template <typename _t>
	static result<resource_tileinfo> get_tiling_info(const _t& resource, const device& device)
	{
		using result_type = result<resource_tileinfo>;
		if (!resource.is_virtual())
			return result_type::make_error("this resource is not virtual!");

		auto dxresource = cast<dx12_resource>(resource.m_native_object);
		if (!dxresource)
			return result_type::make_error("resource.m_native_object failed casting to dx12_resource!");

		auto dxdevice = cast<dx12_device>(device.m_native_object);
		if (!dxdevice)
			return result_type::make_error("device.m_native_object failed casting to dx12_device!");

		D3D12_RESOURCE_DESC desc = dxresource->GetDesc();
		const uint32 plane_count = D3D12GetFormatPlaneCount(dxdevice.get(), desc.Format);
		const uint32 num_subresources = desc.MipLevels * desc.DepthOrArraySize * plane_count;

		resource_tileinfo info{};
		uint32 num_subresource_tilings = num_subresources;
		uint32 total_num_tiles = 0u;
		D3D12_PACKED_MIP_INFO mip_info;
		vector<D3D12_SUBRESOURCE_TILING> subresource_tilings;
		subresource_tilings.resize(num_subresources);
		D3D12_TILE_SHAPE tile_shape;

		dxdevice.get()->GetResourceTiling(dxresource.get(),
			&total_num_tiles,
			&mip_info,
			&tile_shape,
			&num_subresource_tilings,
			0u,
			subresource_tilings.data());

		info.m_num_packed_mips = mip_info.NumPackedMips;
		info.m_num_tiles_total = total_num_tiles;
		info.m_texels_per_tile = { tile_shape.WidthInTexels, tile_shape.HeightInTexels, tile_shape.DepthInTexels };
		for (uint32 i = 0u; i < num_subresource_tilings; ++i)
		{
			resource_tileinfo::per_subresource subtiling{};
			subtiling.m_dimension_in_tiles = { subresource_tilings[i].WidthInTiles, subresource_tilings[i].HeightInTiles, subresource_tilings[i].DepthInTiles };
			subtiling.m_tile_offset = subresource_tilings[i].StartTileIndexInOverallResource;
			info.m_subresource_tilings.push_back(subtiling);
		}
		return info;
	}

	result<resource_tileinfo> texture::get_tiling_info(const device& device) const
	{
		return rhi::get_tiling_info<rhi::texture>(*this, device);
	}

	// [swapchain - interface]
	result<> swapchain::acquire_backbuffer(native_device device)
	{
		return {};
	}
	result<> swapchain::present(const present_args& args) const
	{
		using result_type = result<>;

		auto dxswapchain = cast<dx12_swapchain>(m_native_object);
		if (!dxswapchain)
			return result_type::make_error("failed casting m_native_object to dx12_swapchain!");

		HRESULT hres = dxswapchain->Present(args.m_sync_interval, args.m_flags);
		return hres_to_result(hres, {});
	}
	result<uint32> swapchain::get_current_backbuffer_index() const
	{
		using result_type = result<uint32>;

		auto dxswapchain = cast<dx12_swapchain>(m_native_object);
		if (!dxswapchain)
			return result_type::make_error("failed casting m_native_object to dx12_swapchain!");

		return dxswapchain->GetCurrentBackBufferIndex();
	}
	result<texture> swapchain::get_backbuffer_resource(uint32 index) const
	{
		using result_type = result<texture>;

		auto dxswapchain = cast<dx12_swapchain>(m_native_object);
		if (!dxswapchain)
			return result_type::make_error("failed casting m_native_object to dx12_swapchain!");

		ID3D12Resource* buffer = nullptr;
		HRESULT res = dxswapchain->GetBuffer(index, IID_PPV_ARGS(&buffer));
		return import_texture(buffer);
	}
	result<texture> swapchain::get_backbuffer_resource(native_device)
	{
		uint32 backbuffer_index = get_current_backbuffer_index().get();
		return get_backbuffer_resource(backbuffer_index);
	}
	result<> swapchain::resize(const math::uint2& new_dim)
	{
		// flag all rtvs as dirty
		for (uint32 i = 0u; i < m_data.m_rtv_dirty_list.size(); ++i)
			m_data.m_rtv_dirty_list[i] = true;

		dx12_swapchain* dxswapchain = (dx12_swapchain*)m_native_object;

		DXGI_SWAP_CHAIN_DESC dxdesc{};
		HRESULT res = dxswapchain->GetDesc(&dxdesc);

		res = dxswapchain->ResizeBuffers(
			dxdesc.BufferCount,
			dxdesc.BufferDesc.Width,
			dxdesc.BufferDesc.Height,
			dxdesc.BufferDesc.Format,
			dxdesc.Flags
		);

		return {};
	}
	bool swapchain::owns_rtvs() const
	{
		return m_create_args.m_own_descriptors;
	}
	result<descriptor> swapchain::get_or_create_backbuffer_rtv(device& device)
	{
		using result_type = result<descriptor>;
		if (!owns_rtvs())
			return result_type::make_error("this swapchain does not own its own rtvs!");

		dx12_device* dxdevice = (dx12_device*)device.m_native_object;
		dx12_descheap* dxheap = (dx12_descheap*)m_data.m_rtv_heap;

		uint32 backbuffer_index = get_current_backbuffer_index().get();
		result<texture> backbuffer = get_backbuffer_resource(device.m_native_object);
		result<descriptor> descriptor = sample_descheap(dxdevice, dxheap, backbuffer_index, true);
		if (!descriptor)
			return result_type::make_error("failed sampling descheap!");

		if (m_data.m_rtv_dirty_list[backbuffer_index] == true)
		{
			device.create_rtv(backbuffer.get(), descriptor.get());
			m_data.m_rtv_dirty_list[backbuffer_index] = false;
		}

		return descriptor;
	}
	bool swapchain::is_swapchain_format_supported(const pixelformat& format)
	{
		const vector<pixelformat>& formats = get_swapchain_supported_formats();
		for (const auto& lformat : formats)
		{
			if (lformat == format) return true;
		}
		return false;
	}
	const vector<pixelformat>& swapchain::get_swapchain_supported_formats()
	{
		static vector<pixelformat> formats{};
		static bool done_once = false;
		if (!done_once)
		{
			formats.push_back(pixelformat::rgba_8_unorm());
			done_once = true;
		}
		return formats;
	}

	static result<descriptor> create_rtvs(device& device, const vector<texture const*>& textures);
	static result<descriptor> create_dsv(device& device, const texture& texture);

	// [commandlist - interface]
	result<> commandlist::renderpass_begin(device& device, const begin_renderpass_args& args)
	{
		using result_type = result<>;
		result_type result{};

		ID3D12GraphicsCommandList7* dxcommandlist = (ID3D12GraphicsCommandList7*)m_native_object;
		if (dxcommandlist == nullptr)
			return result_type::make_error("failed");

		// this is where the logic gets a bit creative...
		// the renderpass object was created with a set of color & depth descriptions.
		// the args contains the resources we'll bind in this renderpass.
		// for the rest of this function:
		// - a description refers to what was specified at renderpass CREATION
		// - a binding refers to the resource we're binding specified in ARGS
		const uint32 num_color_targets = args.get_num_color_targets();
		const auto& depth_target = args.m_depth_target;
		const bool has_depth = args.m_depth_target != nullptr && args.m_depth_target->is_valid();

		// create a rtv descriptor for each color binding
		descriptor rtv_descriptor_range{};
		{
			vector<texture const*> color_targets;
			for (uint32 i = 0u; i < k_max_num_rendertargets_per_draw; ++i)
			{
				if (args.m_color_targets[i] != nullptr && args.m_color_targets[i]->is_valid())
					color_targets.push_back(args.m_color_targets[i]);
			}

			auto descriptors = create_rtvs(device, color_targets);
			if (!descriptors)
				return result_type::make_error("failed creating render target view for texture");

			rtv_descriptor_range = descriptors.get();
		}
		
		uint32 rtv_index = 0u;
		vector<D3D12_RENDER_PASS_RENDER_TARGET_DESC> rtvs{};
		const uint32 rtv_stride = device.m_data.get_descriptor_stride(e_descriptor_heap_type::rtv);
		for (uint32 i = 0u; i < k_max_num_rendertargets_per_draw; ++i)
		{
			const auto& attachment = args.m_color_attachments[i];
			if (attachment.m_is_enabled == false)
				continue;

			D3D12_RENDER_PASS_RENDER_TARGET_DESC rtv_desc{};
			rtv_desc.cpuDescriptor.ptr = (SIZE_T)rtv_descriptor_range.m_cpu_address + (rtv_index++ * rtv_stride);
			rtv_desc.BeginningAccess = translate(attachment.m_load);
			rtv_desc.EndingAccess = translate(attachment.m_store);

			if (attachment.m_load == e_load_op::preserve)
			{
				rtv_desc.BeginningAccess.PreserveLocal.AdditionalHeight = 0u;
				rtv_desc.BeginningAccess.PreserveLocal.AdditionalWidth = 0u;
			}
			if (attachment.m_load == e_load_op::clear)
			{
				rtv_desc.BeginningAccess.Clear.ClearValue.Format = translate(attachment.m_format);
				memcpy(rtv_desc.BeginningAccess.Clear.ClearValue.Color, attachment.m_clear.m_data, sizeof(FLOAT[4]));
			}
			if (attachment.m_store == e_store_op::resolve)
			{
				const auto& resolve = attachment.m_resolve;
				rtv_desc.EndingAccess.Resolve.Format = translate(attachment.m_format);
				// rtv_desc.EndingAccess.Resolve.pSrcResource = resolve.m_source->get_native<ID3D12Resource>();
				// rtv_desc.EndingAccess.Resolve.pDstResource = resolve.m_dest->get_native<ID3D12Resource>();
				rtv_desc.EndingAccess.Resolve.PreserveResolveSource = resolve.m_keep_source;
				rtv_desc.EndingAccess.Resolve.pSubresourceParameters;
				rtv_desc.EndingAccess.Resolve.ResolveMode = D3D12_RESOLVE_MODE_MIN;
				rtv_desc.EndingAccess.Resolve.pSubresourceParameters;
				rtv_desc.EndingAccess.Resolve.SubresourceCount = 0u;
			}
			if (attachment.m_store == e_store_op::preserve)
			{
				rtv_desc.EndingAccess.PreserveLocal.AdditionalHeight = 0u;
				rtv_desc.EndingAccess.PreserveLocal.AdditionalWidth = 0u;
			}
			rtvs.push_back(rtv_desc);
		}

		D3D12_RENDER_PASS_DEPTH_STENCIL_DESC* dsv = nullptr;
		const auto& depth_attachment = args.m_depth_attachment;
		if (depth_attachment.m_is_enabled)
		{
			D3D12_RENDER_PASS_DEPTH_STENCIL_DESC dsv_desc{};

			auto descriptor = create_dsv(device, *args.m_depth_target);
			if (!descriptor)
				return result_type::make_error("failed creating depth stencil view for texture");

			dsv_desc.cpuDescriptor.ptr		= (SIZE_T)descriptor.get().m_cpu_address;
			dsv_desc.DepthBeginningAccess	= translate(depth_attachment.m_depth_load);
			dsv_desc.StencilBeginningAccess = translate(depth_attachment.m_stencil_load);
			dsv_desc.DepthEndingAccess		= translate(depth_attachment.m_depth_store);
			dsv_desc.StencilEndingAccess	= translate(depth_attachment.m_stencil_store);
			dsv_desc.StencilBeginningAccess.Clear.ClearValue.DepthStencil.Stencil = depth_attachment.m_stencil_clear;
			dsv_desc.DepthBeginningAccess.Clear.ClearValue.DepthStencil.Depth = depth_attachment.m_depth_clear;
			dsv_desc.DepthBeginningAccess.Clear.ClearValue.Format = translate(depth_attachment.m_format);
			dsv = &dsv_desc;
		}

		D3D12_RENDER_PASS_FLAGS flags = translate(args.m_flags);
		dxcommandlist->BeginRenderPass((uint32)rtvs.size(), rtvs.data(), dsv, flags);

		// m_is_in_renderpass = true;
		return {};
	}
	result<> commandlist::renderpass_end()
	{
		using result_type = result<>;

		ID3D12GraphicsCommandList7* dxcommandlist = (ID3D12GraphicsCommandList7*)m_native_object;
		if (dxcommandlist == nullptr)
			return result_type::make_error("failed");

		dxcommandlist->EndRenderPass();
		return {};
	}
	result<> commandlist::start(device& device)
	{
		if (m_data.m_current_pool == nullptr)
		{
			// create a new command pool (allocator)
			commandpool_create_args args{};
			args.m_type = m_create_args.m_type;
			args.m_device = (native_device)device.m_native_object;
			auto new_alloc_res = device.create(args);
			if (!new_alloc_res) 
				return result<>::make_error("failed creating new commandpool");

			return start((native_commandpool)new_alloc_res.get().m_native_object);
		}
		else
		{
			return start(m_data.m_current_pool);
		}
	}
	result<> commandlist::start(native_commandpool pool)
	{
		using result_type = result<>;
		auto dxallocator = cast<dx12_allocator>(pool);
		if (!dxallocator)
			return result_type::make_error("failed casting pool to dx12_allocator!");

		auto dxcommandlist = cast<dx12_commandlist>(m_native_object);
		if (!dxcommandlist)
			return result_type::make_error("failed casting this to dx12_commandlist!");

		// HRESULT res = dxallocator->Reset();
		// HRESULT hres = dxcommandlist.get()->Close();

		m_data.m_state = e_commandlist_state::recording;

		ID3D12PipelineState* dxinitpipeline = NULL;
		HRESULT
		hres = dxcommandlist->Reset(dxallocator.get(), dxinitpipeline);
		return {};
	}
	result<> commandlist::end()
	{
		ID3D12GraphicsCommandList* dxcommandlist = (ID3D12GraphicsCommandList*)m_native_object;
		HRESULT res = dxcommandlist->Close();
		m_data.m_state = e_commandlist_state::closed;
		return {};
	}
	result<> commandlist::submit(queue& queue)
	{
		return queue.submit({ this });
	}
	result<> commandlist::wait_for_finish() const
	{
		if (m_create_args.m_own_fence && m_data.m_fence != nullptr)
		{
			const uint32 max_value = 64 * 1024 * 1024;
			uint32 i = 0u;

			dx12_fence* dxfence = (dx12_fence*)m_data.m_fence;
			uint32 complete_value = m_data.m_complete_value;

			while (i < max_value)
			{
				const uint64 fence_value = dxfence->GetCompletedValue();
				if (fence_value >= complete_value) return {};

				i++;
			}
		}
		return {};
	}
	bool commandlist::has_fence() const
	{
		return m_create_args.m_own_fence && m_data.m_fence != nullptr;
	}
	result<> commandlist::transition(texture& texture, e_resource_state new_state)
	{
		using result_type = result<>;

		dx12_resource* dxresource = (dx12_resource*)texture.m_native_object;
		dx12_commandlist* dxcmdlist = (dx12_commandlist*)m_native_object;
		
		const e_resource_state old_state = texture.get_resource_state();
		if (new_state == old_state)
			return result<>::make_error("transition to same state is considered a no-op!");

		// setup the barrier
		D3D12_RESOURCE_BARRIER barrier{};
		barrier.Transition.pResource = dxresource;
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags;
		barrier.Transition.StateAfter = translate(new_state);
		barrier.Transition.StateBefore = translate(old_state);
		barrier.Transition.Subresource = 0u;
		dxcmdlist->ResourceBarrier(1u, &barrier);

		// update the state
		texture.m_data.m_previous_state = texture.m_data.m_previous_state;
		texture.m_data.m_current_state = new_state;
		return {};
	}
	result<> commandlist::clear_rtv(descriptor rtv, const clear& clear)
	{
		dx12_commandlist* dxcmdlist = (dx12_commandlist*)m_native_object;
		D3D12_CPU_DESCRIPTOR_HANDLE dxdescriptor{ .ptr = rtv.m_cpu_address };
		dxcmdlist->ClearRenderTargetView(dxdescriptor, clear.m_colour.data(), 0u, NULL);
		return {};
	}
	result<> commandlist::copy(texture& source, texture& dest)
	{
		using result_type = result<>;
		if (source.get_width() != dest.get_width())
			return result_type::make_error("cannot copy texture of different width!");
		if (source.get_height() != dest.get_height())
			return result_type::make_error("cannot copy texture of different height!");
		if (source.get_depth() != dest.get_depth())
			return result_type::make_error("cannot copy texture of different depth!");

		auto dxsource = cast<dx12_resource>(source.m_native_object);
		if (!dxsource) 
			return result_type::make_error("failed casting source, to dx12_resource");
		auto dxdest = cast<dx12_resource>(dest.m_native_object);
		if (!dxdest) 
			return result_type::make_error("failed casting dest to dx12_resource");
		auto dxcmdlist = cast<dx12_commandlist>(m_native_object);
		if (!dxcmdlist) 
			return result_type::make_error("failed casting m_native to dx12_commandlist");

		dxcmdlist->CopyResource(dxdest.get(), dxsource.get());
		return {};
	}
	result<> commandlist::copy(buffer& source, buffer& dest)
	{
		using result_type = result<>;

		if (source.get_bytesize() != dest.get_bytesize())
			return result_type::make_error("cannot copy buffers of different bytesize!");
	
		auto dxsource = cast<dx12_resource>(source.m_native_object);
		if (!dxsource)
			return result_type::make_error("failed casting source, to dx12_resource");
		auto dxdest = cast<dx12_resource>(dest.m_native_object);
		if (!dxdest)
			return result_type::make_error("failed casting dest to dx12_resource");
		auto dxcmdlist = cast<dx12_commandlist>(m_native_object);
		if (!dxcmdlist)
			return result_type::make_error("failed casting m_native to dx12_commandlist");

		dxcmdlist->CopyResource(dxdest.get(), dxsource.get());
		return {};
	}

	inline static result<descriptor> create_rtvs(device& device, const vector<texture const*>& textures)
	{
		// 1. create RTV on the internal heap
		dx12_device* dxdevice = device.m_native_object;
		
		D3D12_CPU_DESCRIPTOR_HANDLE base = device.m_data.m_internal_rtv_heap->GetCPUDescriptorHandleForHeapStart();
		D3D12_CPU_DESCRIPTOR_HANDLE handle = base;
		for (uint32 i = 0u; i < textures.size(); ++i)
		{
			texture const* texture = textures[i];
			if (texture == nullptr) continue;

			dx12_resource* dxresource = texture->m_native_object;

			D3D12_RENDER_TARGET_VIEW_DESC rtv_desc{};
			const uint32 arraysize = texture->get_arraysize();
			rtv_desc.ViewDimension = translate_rtv(texture->get_texture_type(), arraysize);
			rtv_desc.Format = translate(texture->get_format());
			const uint32 mipslice = 0u;
			const uint32 planeslice = 0u;
			const uint32 firstarrayslice = 0u;
			switch (rtv_desc.ViewDimension)
			{
			case D3D12_RTV_DIMENSION_TEXTURE1D:
				rtv_desc.Texture1D.MipSlice = mipslice;
				break;
			case D3D12_RTV_DIMENSION_TEXTURE2D:
				rtv_desc.Texture2D.MipSlice = mipslice;
				rtv_desc.Texture2D.PlaneSlice = planeslice;
				break;
			case D3D12_RTV_DIMENSION_TEXTURE1DARRAY:
				rtv_desc.Texture1DArray.ArraySize = arraysize;
				rtv_desc.Texture1DArray.FirstArraySlice = firstarrayslice;
				rtv_desc.Texture1DArray.MipSlice = mipslice;
				break;
			case D3D12_RTV_DIMENSION_TEXTURE2DARRAY:
				rtv_desc.Texture2DArray.ArraySize = arraysize;
				rtv_desc.Texture2DArray.FirstArraySlice = firstarrayslice;
				rtv_desc.Texture2DArray.MipSlice = mipslice;
				rtv_desc.Texture2DArray.PlaneSlice = planeslice;
				break;
			case D3D12_RTV_DIMENSION_TEXTURE3D:
				rtv_desc.Texture3D.FirstWSlice = firstarrayslice;
				rtv_desc.Texture3D.MipSlice = mipslice;
				rtv_desc.Texture3D.WSize = texture->get_depth();
				break;
			}
			dxdevice->CreateRenderTargetView(dxresource, &rtv_desc, handle);
			
			handle.ptr += device.m_data.get_descriptor_stride(e_descriptor_heap_type::rtv);
		}

		descriptor result{};
		result.m_cpu_address = base.ptr;
		return result;
	}
	inline static result<descriptor> create_dsv(device& device, const texture& texture)
	{
		// 1. create RTV on the internal heap
		dx12_device* dxdevice = device.m_native_object;
		dx12_resource* dxresource = texture.m_native_object;

		D3D12_CPU_DESCRIPTOR_HANDLE handle = device.m_data.m_internal_dsv_heap->GetCPUDescriptorHandleForHeapStart();
		D3D12_DEPTH_STENCIL_VIEW_DESC dsv_desc{};
		const uint32 arraysize = texture.get_arraysize();
		dsv_desc.ViewDimension = translate_dsv(texture.get_texture_type(), arraysize);
		dsv_desc.Format = translate(texture.get_format());
		dsv_desc.Flags;
		const uint32 mipslice = 0u;
		const uint32 planeslice = 0u;
		const uint32 firstarrayslice = 0u;
		switch (dsv_desc.ViewDimension)
		{
		case D3D12_DSV_DIMENSION_TEXTURE1D:
			dsv_desc.Texture1D.MipSlice = mipslice;
			break;
		case D3D12_DSV_DIMENSION_TEXTURE2D:
			dsv_desc.Texture2D.MipSlice = mipslice;
			break;
		case D3D12_DSV_DIMENSION_TEXTURE1DARRAY:
			dsv_desc.Texture1DArray.ArraySize = arraysize;
			dsv_desc.Texture1DArray.FirstArraySlice = firstarrayslice;
			dsv_desc.Texture1DArray.MipSlice = mipslice;
			break;
		case D3D12_DSV_DIMENSION_TEXTURE2DARRAY:
			dsv_desc.Texture2DArray.ArraySize = arraysize;
			dsv_desc.Texture2DArray.FirstArraySlice = firstarrayslice;
			dsv_desc.Texture2DArray.MipSlice = mipslice;
			break;
		}
		dxdevice->CreateDepthStencilView(dxresource, &dsv_desc, handle);

		descriptor result{};
		result.m_cpu_address = handle.ptr;
		return result;
	}
	
	result<> commandlist::clear_texture(device& device, const texture& texture, const clear& clear)
	{
		using result_type = result<>;
		dx12_descheap* internal_rtv_heap = device.m_data.m_internal_rtv_heap;
		if (internal_rtv_heap == nullptr)
			return result_type::make_error("device.m_data.m_internal_rtv_heap is nullptr!");

		auto new_rtv = create_rtvs(device, { &texture });
		if (!new_rtv)
			return result_type::make_error("failed creating RTV");

		dx12_commandlist* dxcommandlist = cast<dx12_commandlist>(m_native_object).get();
		float color[4u] = { clear.m_colour.x, clear.m_colour.y, clear.m_colour.z, clear.m_colour.w };
		D3D12_CPU_DESCRIPTOR_HANDLE handle = { .ptr = new_rtv.get().m_cpu_address };
		dxcommandlist->ClearRenderTargetView(handle, color, 0u, nullptr);
		return {};
	}
	result<> commandlist::bind_descheaps(descheap const* resource_heap, descheap const* sampler_heap)
	{
		using result_type = result<>;
		auto dxcmdlist = cast<dx12_commandlist>(m_native_object);
		if (!dxcmdlist)
			return result_type::make_error("failed casting m_native to dx12_commandlist");

		vector<dx12_descheap*> descheaps;
		if (resource_heap) descheaps.push_back(resource_heap->m_native_object);
		if (sampler_heap) descheaps.push_back(sampler_heap->m_native_object);

		if (descheaps.size() == 0u)
			return result_type::make_error("no valid descheaps were passed!");

		dxcmdlist->SetDescriptorHeaps(static_cast<uint32>(descheaps.size()), descheaps.data());
		return {};
	}
	result<> commandlist::bind_rootsignature(const rootsignature& signature, bool is_compute)
	{
		using result_type = result<>;
		auto dxcmdlist = cast<dx12_commandlist>(m_native_object);
		if (!dxcmdlist)
			return result_type::make_error("failed casting m_native to dx12_commandlist");

		auto dxsignature = cast<dx12_rootsignature>(signature.m_native_object);
		if (!dxsignature)
			return result_type::make_error("failed casting signature.m_native_object to dx12_rootsignature");

		if (is_compute) dxcmdlist->SetComputeRootSignature(dxsignature.get());
		else dxcmdlist->SetGraphicsRootSignature(dxsignature.get());
		return {};
	}
	result<> commandlist::bind_pipeline(const pipeline& pipeline)
	{
		using result_type = result<>;
		auto dxcmdlist = cast<dx12_commandlist>(m_native_object);
		if (!dxcmdlist)
			return result_type::make_error("failed casting m_native to dx12_commandlist");

		auto dxpipeline = cast<dx12_pipeline>(pipeline.m_native_object);
		if (!dxpipeline)
			return result_type::make_error("failed casting pipeline.m_native to dx12_pipeline");

		if (pipeline.is_graphics())
			set_primitive_topology(pipeline.m_create_args.m_graphics.m_primitive_topology);

		dxcmdlist->SetPipelineState(dxpipeline.get());
		return {};
	}
	result<> commandlist::bind_texture_uav(const texture& texture, uint32 param_index, bool is_compute)
	{
		using result_type = result<>;
		auto dxcmdlist = cast<dx12_commandlist>(m_native_object);
		if (!dxcmdlist)
			return result_type::make_error("failed casting m_native to dx12_commandlist");
		
		auto dxresource = cast<dx12_resource>(texture.m_native_object);
		if (!dxresource)
			return result_type::make_error("texture.m_native_object failed casting to dx12_resource!");

		// dxcmdlist->SetGraphicsRootDescriptorTable(param_index, )
		D3D12_GPU_VIRTUAL_ADDRESS gpu_address = dxresource->GetGPUVirtualAddress();
		if (is_compute) dxcmdlist->SetComputeRootUnorderedAccessView(param_index, gpu_address);
		else dxcmdlist->SetGraphicsRootUnorderedAccessView(param_index, gpu_address);
		
		return {};
	}
	result<> commandlist::bind_texture_srv(const texture& texture, uint32 param_index, bool is_compute)
	{
		using result_type = result<>;
		auto dxcmdlist = cast<dx12_commandlist>(m_native_object);
		if (!dxcmdlist)
			return result_type::make_error("failed casting m_native to dx12_commandlist");

		auto dxresource = cast<dx12_resource>(texture.m_native_object);
		if (!dxresource)
			return result_type::make_error("texture.m_native_object failed casting to dx12_resource!");

		D3D12_GPU_VIRTUAL_ADDRESS gpu_address = dxresource->GetGPUVirtualAddress();
		if (is_compute) dxcmdlist->SetComputeRootShaderResourceView(param_index, gpu_address);
		else dxcmdlist->SetGraphicsRootShaderResourceView(param_index, gpu_address);
		return {};
	}
	result<> commandlist::bind_buffer_uav(const buffer& buffer, uint32 param_index, bool is_compute)
	{
		using result_type = result<>;
		auto dxcmdlist = cast<dx12_commandlist>(m_native_object);
		if (!dxcmdlist)
			return result_type::make_error("failed casting m_native to dx12_commandlist");

		auto dxresource = cast<dx12_resource>(buffer.m_native_object);
		if (!dxresource)
			return result_type::make_error("buffer.m_native_object failed casting to dx12_resource!");

		D3D12_GPU_VIRTUAL_ADDRESS gpu_address = dxresource->GetGPUVirtualAddress();
		if (is_compute) dxcmdlist->SetComputeRootUnorderedAccessView(param_index, gpu_address);
		else dxcmdlist->SetGraphicsRootUnorderedAccessView(param_index, gpu_address);
		return {};
	}
	result<> commandlist::bind_buffer_srv(const buffer& buffer, uint32 param_index, bool is_compute)
	{
		using result_type = result<>;
		auto dxcmdlist = cast<dx12_commandlist>(m_native_object);
		if (!dxcmdlist)
			return result_type::make_error("failed casting m_native to dx12_commandlist");

		auto dxresource = cast<dx12_resource>(buffer.m_native_object);
		if (!dxresource)
			return result_type::make_error("buffer.m_native_object failed casting to dx12_resource!");

		D3D12_GPU_VIRTUAL_ADDRESS gpu_address = dxresource->GetGPUVirtualAddress();
		if (is_compute) dxcmdlist->SetComputeRootShaderResourceView(param_index, gpu_address);
		else dxcmdlist->SetGraphicsRootShaderResourceView(param_index, gpu_address);
		return {};
	}
	result<> commandlist::bind_buffer_cbv(const buffer& buffer, uint32 param_index, bool is_compute)
	{
		using result_type = result<>;
		auto dxcmdlist = cast<dx12_commandlist>(m_native_object);
		if (!dxcmdlist)
			return result_type::make_error("failed casting m_native to dx12_commandlist");

		auto dxresource = cast<dx12_resource>(buffer.m_native_object);
		if (!dxresource)
			return result_type::make_error("buffer.m_native_object failed casting to dx12_resource!");

		D3D12_GPU_VIRTUAL_ADDRESS gpu_address = dxresource->GetGPUVirtualAddress();
		if (is_compute) dxcmdlist->SetComputeRootConstantBufferView(param_index, gpu_address);
		else dxcmdlist->SetGraphicsRootConstantBufferView(param_index, gpu_address);
		return {};
	}
	
	result<> commandlist::set_rendertargets(device& dev, 
		const vector<texture const*>& color_targets, 
		texture const* depth_target)
	{
		using result_type = result<>;
		auto dxcmdlist = cast<dx12_commandlist>(m_native_object);
		if (!dxcmdlist)
			return result_type::make_error("failed casting m_native to dx12_commandlist");

		uint32 num_valid_targets = 0u;
		for (const auto& target : color_targets)
			if (target != nullptr) num_valid_targets++;
		
		D3D12_CPU_DESCRIPTOR_HANDLE rtv_range{};
		if (num_valid_targets > 0u)
		{
			auto rtvs = create_rtvs(dev, color_targets);
			if (!rtvs)
				return result_type::make_error("failed creating rtvs!");
			rtv_range = { .ptr = rtvs.get().m_cpu_address };
		}

		const bool has_depth = depth_target != nullptr;
		if (!has_depth && num_valid_targets == 0u)
			return result_type::make_warning({}, "noop");

		D3D12_CPU_DESCRIPTOR_HANDLE dsv_range{};
		if (has_depth)
		{
			auto dsv = create_dsv(dev, *depth_target);
			if (!dsv)
				return result_type::make_error("failed creating dsv!");
			dsv_range = { .ptr = dsv.get().m_cpu_address };
		}
		dxcmdlist->OMSetRenderTargets(num_valid_targets, &rtv_range, true, &dsv_range);
	}
	result<> commandlist::bind_vertexbuffer(const buffer& vertexbuffer)
	{
		using result_type = result<>;
		auto dxcmdlist = cast<dx12_commandlist>(m_native_object);
		if (!dxcmdlist)
			return result_type::make_error("failed casting m_native to dx12_commandlist");

		auto dxresource = cast<dx12_resource>(vertexbuffer.m_native_object);
		if (!dxresource)
			return result_type::make_error("vertexbuffer.m_native_object failed casting to dx12_resource!");

		D3D12_VERTEX_BUFFER_VIEW dxview{};
		dxview.BufferLocation = dxresource->GetGPUVirtualAddress();
		dxview.SizeInBytes = (uint32)vertexbuffer.get_bytesize();
		dxview.StrideInBytes = (uint32)vertexbuffer.get_bytestride();
		dxcmdlist->IASetVertexBuffers(0u, 1u, &dxview);
		return {};
	}
	result<> commandlist::bind_indexbuffer(const buffer& indexbuffer)
	{
		using result_type = result<>;
		auto dxcmdlist = cast<dx12_commandlist>(m_native_object);
		if (!dxcmdlist)
			return result_type::make_error("failed casting m_native to dx12_commandlist");

		auto dxresource = cast<dx12_resource>(indexbuffer.m_native_object);
		if (!dxresource)
			return result_type::make_error("indexbuffer.m_native_object failed casting to dx12_resource!");

		D3D12_INDEX_BUFFER_VIEW dxview{};
		dxview.BufferLocation = dxresource->GetGPUVirtualAddress();
		dxview.Format = DXGI_FORMAT_R32_UINT;
		dxview.SizeInBytes = (uint32)indexbuffer.get_bytesize();
		dxcmdlist->IASetIndexBuffer(&dxview);
		return {};
	}
	result<> commandlist::draw(const draw_args& args)
	{
		using result_type = result<>;
		auto dxcmdlist = cast<dx12_commandlist>(m_native_object);
		if (!dxcmdlist)
			return result_type::make_error("failed casting m_native to dx12_commandlist");

		dxcmdlist->DrawInstanced(args.m_num_vertices, args.m_num_instances, args.m_start_vertex, args.m_start_instance);
		return {};
	}
	result<> commandlist::draw_indexed(const draw_indexed_args& args)
	{
		using result_type = result<>;
		auto dxcmdlist = cast<dx12_commandlist>(m_native_object);
		if (!dxcmdlist)
			return result_type::make_error("failed casting m_native to dx12_commandlist");

		dxcmdlist->DrawIndexedInstanced(
			args.m_num_indices, 
			args.m_num_instances, 
			args.m_start_index, 
			args.m_start_vertex,
			args.m_start_instance);
		return {};
	}
	result<> commandlist::dispatch(const math::uint3& group_nums)
	{
		using result_type = result<>;
		auto dxcmdlist = cast<dx12_commandlist>(m_native_object);
		if (!dxcmdlist)
			return result_type::make_error("failed casting m_native to dx12_commandlist");

		dxcmdlist->Dispatch(group_nums.x, group_nums.y, group_nums.z);
		return {};
	}
	result<> commandlist::set_primitive_topology(e_primitive_topology type)
	{
		using result_type = result<>;
		auto dxcmdlist = cast<dx12_commandlist>(m_native_object);
		if (!dxcmdlist)
			return result_type::make_error("failed casting m_native to dx12_commandlist");

		dxcmdlist->IASetPrimitiveTopology(translate(type));
		return {};
	}

	// [descheap]
	result<uint32> descheap::allocate(uint32 num_descriptors)
	{
		using result_type = result<uint32>;

		ID3D12DescriptorHeap* dxheap = (ID3D12DescriptorHeap*)m_native_object;

		auto& freelist = m_data.m_freelist;
		for (uint32 i = 0u; i < freelist.size(); ++i)
		{
			// skip non-free ones
			if (is_allocated(i)) continue;

			bool all_neighbours_free = true;
			for (uint32 x = 0u; x < num_descriptors; ++x)
			{
				all_neighbours_free &= !is_allocated(i + x);
			}

			if (all_neighbours_free)
			{
				// set all allocated descriptors unfree
				for (uint32 x = 0u; x < num_descriptors; ++x)
					freelist[i + x] = false;

				// return base index
				return i;
			}
		}

		return result_type::make_error("no free ranges found!");
	}
	bool descheap::is_allocated(uint32 index) const
	{
		if (index >= m_data.m_freelist.size())
			return false;

		return m_data.m_freelist[index];
	}
	result<> descheap::free(const vector<descriptor_range>& ranges)
	{
		ID3D12DescriptorHeap* dxheap = (ID3D12DescriptorHeap*)m_native_object;
		return {};
	}
	result<> descheap::free(const descriptor& desc)
	{
		return {};
	}
	result<> descheap::free(const uint32 index)
	{
		using result_type = result<>;
		auto dxheap = cast<dx12_descheap>(m_native_object);
		if (!dxheap) return result_type::make_error("failed casting m_native to dx12_heap");

		m_data.m_freelist[index] = true;
		return {};
	}
	result<descriptor> descheap::get_cpu_descriptor(uint32 index) const
	{
		dx12_descheap* dxheap = (dx12_descheap*)m_native_object;
		return sample_descheap(dxheap, m_data.m_descriptor_stride, index, true);
	}
	result<descriptor> descheap::get_gpu_descriptor(uint32 index) const
	{
		dx12_descheap* dxheap = (dx12_descheap*)m_native_object;
		return sample_descheap(dxheap, m_data.m_descriptor_stride, index, false);
	}

	// [texture2D]
	result<> texture::transition(commandlist& cmdlist, e_resource_state new_state)
	{
		return cmdlist.transition(*this, new_state);
	}

	void static_pixel_formats::initialize()
	{
		if (!m_initialized)
		{
			using namespace format;
			m_initialized = true;
			m_formats[DXGI_FORMAT_UNKNOWN]					= {"DXGI_FORMAT_UNKNOWN", {}};
			m_formats[DXGI_FORMAT_R32G32B32A32_TYPELESS]	= {"DXGI_FORMAT_R32G32B32A32_TYPELESS",		{ e_format::typeless,	{_r,_32}, {_g,_32}, {_b,_32}, {_a,_32} }};
			m_formats[DXGI_FORMAT_R32G32B32A32_FLOAT]		= {"DXGI_FORMAT_R32G32B32A32_FLOAT",		{ e_format::sfloat,		{_r,_32}, {_g,_32}, {_b,_32}, {_a,_32} }};
			m_formats[DXGI_FORMAT_R32G32B32A32_UINT]		= {"DXGI_FORMAT_R32G32B32A32_UINT",			{ e_format::uint,		{_r,_32}, {_g,_32}, {_b,_32}, {_a,_32} }};
			m_formats[DXGI_FORMAT_R32G32B32A32_SINT]		= {"DXGI_FORMAT_R32G32B32A32_SINT",			{ e_format::sint,		{_r,_32}, {_g,_32}, {_b,_32}, {_a,_32} }};
			m_formats[DXGI_FORMAT_R32G32B32_TYPELESS]		= {"DXGI_FORMAT_R32G32B32_TYPELESS",		{ e_format::typeless,	{_r,_32}, {_g,_32}, {_b,_32} }};
			m_formats[DXGI_FORMAT_R32G32B32_FLOAT]			= {"DXGI_FORMAT_R32G32B32_FLOAT",			{ e_format::sfloat,		{_r,_32}, {_g,_32}, {_b,_32} }};
			m_formats[DXGI_FORMAT_R32G32B32_UINT]			= {"DXGI_FORMAT_R32G32B32_UINT",			{ e_format::uint,		{_r,_32}, {_g,_32}, {_b,_32} }};
			m_formats[DXGI_FORMAT_R32G32B32_SINT]			= {"DXGI_FORMAT_R32G32B32_SINT",			{ e_format::sint,		{_r,_32}, {_g,_32}, {_b,_32} }};
			m_formats[DXGI_FORMAT_R16G16B16A16_TYPELESS]	= {"DXGI_FORMAT_R16G16B16A16_TYPELESS",		{ e_format::typeless,	{_r,_16}, {_g,_16}, {_b,_16}, {_a,_16} }};
			m_formats[DXGI_FORMAT_R16G16B16A16_FLOAT]		= {"DXGI_FORMAT_R16G16B16A16_FLOAT",		{ e_format::sfloat,		{_r,_16}, {_g,_16}, {_b,_16}, {_a,_16} }};
			m_formats[DXGI_FORMAT_R16G16B16A16_UNORM]		= {"DXGI_FORMAT_R16G16B16A16_UNORM",		{ e_format::unorm,		{_r,_16}, {_g,_16}, {_b,_16}, {_a,_16} }};
			m_formats[DXGI_FORMAT_R16G16B16A16_UINT]		= {"DXGI_FORMAT_R16G16B16A16_UINT",			{ e_format::uint,		{_r,_16}, {_g,_16}, {_b,_16}, {_a,_16} }};
			m_formats[DXGI_FORMAT_R16G16B16A16_SNORM]		= {"DXGI_FORMAT_R16G16B16A16_SNORM",		{ e_format::snorm,		{_r,_16}, {_g,_16}, {_b,_16}, {_a,_16} }};
			m_formats[DXGI_FORMAT_R16G16B16A16_SINT]		= {"DXGI_FORMAT_R16G16B16A16_SINT",			{ e_format::sint,		{_r,_16}, {_g,_16}, {_b,_16}, {_a,_16} }};
			m_formats[DXGI_FORMAT_R32G32_TYPELESS]			= {"DXGI_FORMAT_R32G32_TYPELESS",			{ e_format::typeless,	{_r,_32}, {_g,_32} }};
			m_formats[DXGI_FORMAT_R32G32_FLOAT]				= {"DXGI_FORMAT_R32G32_FLOAT",				{ e_format::sfloat,		{_r,_32}, {_g,_32} }};
			m_formats[DXGI_FORMAT_R32G32_UINT]				= {"DXGI_FORMAT_R32G32_UINT",				{ e_format::uint,		{_r,_32}, {_g,_32} }};
			m_formats[DXGI_FORMAT_R32G32_SINT]				= {"DXGI_FORMAT_R32G32_SINT",				{ e_format::sint,		{_r,_32}, {_g,_32} }};
			m_formats[DXGI_FORMAT_R32G8X24_TYPELESS]		= {"DXGI_FORMAT_R32G8X24_TYPELESS",			{ e_format::typeless,	{_r,_32}, {_g,_8}, {_x,_24,typeless} }};
			m_formats[DXGI_FORMAT_D32_FLOAT_S8X24_UINT]		= {"DXGI_FORMAT_D32_FLOAT_S8X24_UINT",		{ e_format::sfloat,		{_d,_32}, {_s,_8}, {_x,_24,uint} }};
			m_formats[DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS] = {"DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS",	{ e_format::sfloat,		{_r,_32}, {_x,_8}, {_x,_24,typeless} }};
			m_formats[DXGI_FORMAT_X32_TYPELESS_G8X24_UINT]	= {"DXGI_FORMAT_X32_TYPELESS_G8X24_UINT",	{ e_format::typeless,	{_x,_32}, {_g,_8}, {_x,_24,uint} }};
			m_formats[DXGI_FORMAT_R10G10B10A2_TYPELESS]		= {"DXGI_FORMAT_R10G10B10A2_TYPELESS",		{ e_format::typeless,	{_r,_10}, {_g,_10}, {_b,_10}, {_a,_2} }};
			m_formats[DXGI_FORMAT_R10G10B10A2_UNORM]		= {"DXGI_FORMAT_R10G10B10A2_UNORM",			{ e_format::unorm,		{_r,_10}, {_g,_10}, {_b,_10}, {_a,_2} }};
			m_formats[DXGI_FORMAT_R10G10B10A2_UINT]			= {"DXGI_FORMAT_R10G10B10A2_UINT",			{ e_format::uint,		{_r,_10}, {_g,_10}, {_b,_10}, {_a,_2} }};
			m_formats[DXGI_FORMAT_R11G11B10_FLOAT]			= {"DXGI_FORMAT_R11G11B10_FLOAT",			{ e_format::sfloat,		{_r,_11}, {_g,_11}, {_b,_10} }};
			m_formats[DXGI_FORMAT_R8G8B8A8_TYPELESS]		= {"DXGI_FORMAT_R8G8B8A8_TYPELESS",			{ e_format::typeless,	{_r,_8}, {_g,_8}, {_b,_8}, {_a,_8} }};
			m_formats[DXGI_FORMAT_R8G8B8A8_UNORM]			= {"DXGI_FORMAT_R8G8B8A8_UNORM",			{ e_format::unorm,		{_r,_8}, {_g,_8}, {_b,_8}, {_a,_8} }};

			m_formats[DXGI_FORMAT_R8G8B8A8_UNORM_SRGB]		= {"", { e_format::unorm_srgb,	{_r,_8}, {_g,_8}, {_b,_8}, {_a,_8} }};
			m_formats[DXGI_FORMAT_R8G8B8A8_UINT]			= {"", { e_format::uint,		{_r,_8}, {_g,_8}, {_b,_8}, {_a,_8} }};
			m_formats[DXGI_FORMAT_R8G8B8A8_SNORM]			= {"", { e_format::snorm,		{_r,_8}, {_g,_8}, {_b,_8}, {_a,_8} }};
			m_formats[DXGI_FORMAT_R8G8B8A8_SINT]			= {"", { e_format::sint,		{_r,_8}, {_g,_8}, {_b,_8}, {_a,_8} }};
			m_formats[DXGI_FORMAT_R16G16_TYPELESS]			= {"", { e_format::typeless,	{_r,_16}, {_g,_16} }};
			m_formats[DXGI_FORMAT_R16G16_FLOAT]				= {"", { e_format::sfloat,		{_r,_16}, {_g,_16} }};
			m_formats[DXGI_FORMAT_R16G16_UNORM]				= {"", { e_format::unorm,		{_r,_16}, {_g,_16} }};
			m_formats[DXGI_FORMAT_R16G16_UINT]				= {"", { e_format::uint,		{_r,_16}, {_g,_16} }};
			m_formats[DXGI_FORMAT_R16G16_SNORM]				= {"", { e_format::snorm,		{_r,_16}, {_g,_16} }};
			m_formats[DXGI_FORMAT_R16G16_SINT]				= {"", { e_format::sint,		{_r,_16}, {_g,_16} }};
			m_formats[DXGI_FORMAT_R32_TYPELESS]				= {"", { e_format::typeless,	{_r,_32} }};
			m_formats[DXGI_FORMAT_D32_FLOAT]				= {"", { e_format::sfloat,		{_d,_32} }};
			m_formats[DXGI_FORMAT_R32_FLOAT]				= {"", { e_format::sfloat,		{_r,_32} }};
			m_formats[DXGI_FORMAT_R32_UINT]					= {"", { e_format::uint,		{_r,_32} }};
			m_formats[DXGI_FORMAT_R32_SINT]					= {"", { e_format::sint,		{_r,_32} }};
			m_formats[DXGI_FORMAT_R24G8_TYPELESS]			= {"", { e_format::typeless,	{_r,_24}, {_g,_8} }};
			m_formats[DXGI_FORMAT_D24_UNORM_S8_UINT]		= {"", { e_format::unorm,		{_d,_24}, {_s,_8,uint} }};
			m_formats[DXGI_FORMAT_R24_UNORM_X8_TYPELESS]	= {"", { e_format::unorm,		{_r,_24}, {_x,_8,typeless} }};
			m_formats[DXGI_FORMAT_X24_TYPELESS_G8_UINT]		= {"", { e_format::typeless,	{_x,_24}, {_g,_8,uint } }};
			m_formats[DXGI_FORMAT_R8G8_TYPELESS]			= {"", { e_format::typeless,	{_r,_8}, {_g,_8} }};
			m_formats[DXGI_FORMAT_R8G8_UNORM]				= {"", { e_format::unorm,		{_r,_8}, {_g,_8} }};
			m_formats[DXGI_FORMAT_R8G8_UINT]				= {"", { e_format::uint,		{_r,_8}, {_g,_8} }};
			m_formats[DXGI_FORMAT_R8G8_SNORM]				= {"", { e_format::snorm,		{_r,_8}, {_g,_8} }};
			m_formats[DXGI_FORMAT_R8G8_SINT]				= {"", { e_format::sint,		{_r,_8}, {_g,_8} }};
			m_formats[DXGI_FORMAT_R16_TYPELESS]				= {"", { e_format::typeless,	{_r,_16} }};
			m_formats[DXGI_FORMAT_R16_FLOAT]				= {"", { e_format::sfloat,		{_r,_16} }};
			m_formats[DXGI_FORMAT_D16_UNORM]				= {"", { e_format::unorm,		{_d,_16} }};
			m_formats[DXGI_FORMAT_R16_UNORM]				= {"", { e_format::unorm,		{_r,_16} }};
			m_formats[DXGI_FORMAT_R16_UINT]					= {"", { e_format::uint,			{_r,_16} }};
			m_formats[DXGI_FORMAT_R16_SNORM]				= {"", { e_format::snorm,		{_r,_16} }};
			m_formats[DXGI_FORMAT_R16_SINT]					= {"", { e_format::sint,			{_r,_16} }};
			m_formats[DXGI_FORMAT_R8_TYPELESS]				= {"", { e_format::typeless,		{_r,_8} }};
			m_formats[DXGI_FORMAT_R8_UNORM]					= {"", { e_format::unorm,		{_r,_8} }};
			m_formats[DXGI_FORMAT_R8_UINT]					= {"", { e_format::uint,			{_r,_8} }};
			m_formats[DXGI_FORMAT_R8_SNORM]					= {"", { e_format::snorm,		{_r,_8} }};
			m_formats[DXGI_FORMAT_R8_SINT]					= {"", { e_format::sint,			{_r,_8} }};
			m_formats[DXGI_FORMAT_A8_UNORM]					= {"", { e_format::unorm,		{_a,_8} }};
			m_formats[DXGI_FORMAT_R1_UNORM]					= {"", { e_format::unorm,		{_r,_1} }};
			m_formats[DXGI_FORMAT_R9G9B9E5_SHAREDEXP]		= {"", { e_format::shared_exp,	{_r,_9}, {_g,_9}, {_b,_9}, {_e,_5 } }};
			m_formats[DXGI_FORMAT_R8G8_B8G8_UNORM]			= {"", { e_format::unorm,		{_r,_8}, {_g,_8}, {_b,_8}, {_g,_8} }};
			m_formats[DXGI_FORMAT_G8R8_G8B8_UNORM]			= {"", { e_format::unorm,		{_g,_8}, {_r,_8}, {_g,_8}, {_b,_8} }};
			m_formats[DXGI_FORMAT_BC1_TYPELESS]				= {"", { e_format::typeless,		e_spec_format::bc1 }};
			m_formats[DXGI_FORMAT_BC1_UNORM]				= {"", { e_format::unorm,		e_spec_format::bc1 }};
			m_formats[DXGI_FORMAT_BC1_UNORM_SRGB]			= {"", { e_format::unorm_srgb,	e_spec_format::bc1 }};
			m_formats[DXGI_FORMAT_BC2_TYPELESS]				= {"", { e_format::typeless,		e_spec_format::bc2 }};
			m_formats[DXGI_FORMAT_BC2_UNORM]				= {"", { e_format::unorm,		e_spec_format::bc2 }};
			m_formats[DXGI_FORMAT_BC2_UNORM_SRGB]			= {"", { e_format::unorm_srgb,	e_spec_format::bc2 }};
			m_formats[DXGI_FORMAT_BC3_TYPELESS]				= {"", { e_format::typeless,		e_spec_format::bc3 }};
			m_formats[DXGI_FORMAT_BC3_UNORM]				= {"", { e_format::unorm,		e_spec_format::bc3 }};
			m_formats[DXGI_FORMAT_BC3_UNORM_SRGB]			= {"", { e_format::unorm_srgb,	e_spec_format::bc3 }};
			m_formats[DXGI_FORMAT_BC4_TYPELESS]				= {"", { e_format::typeless,		e_spec_format::bc4 }};
			m_formats[DXGI_FORMAT_BC4_UNORM]				= {"", { e_format::unorm,		e_spec_format::bc4 }};
			m_formats[DXGI_FORMAT_BC4_SNORM]				= {"", { e_format::snorm,		e_spec_format::bc4 }};
			m_formats[DXGI_FORMAT_BC5_TYPELESS]				= {"", { e_format::typeless,		e_spec_format::bc5 }};
			m_formats[DXGI_FORMAT_BC5_UNORM]				= {"", { e_format::unorm,		e_spec_format::bc5 }};
			m_formats[DXGI_FORMAT_BC5_SNORM]				= {"", { e_format::snorm,		e_spec_format::bc5 }};
			m_formats[DXGI_FORMAT_BC6H_TYPELESS]			= {"", { e_format::typeless,		e_spec_format::bc6h }};
			m_formats[DXGI_FORMAT_BC6H_UF16]				= {"", { e_format::ufloat,		e_spec_format::bc6h }};
			m_formats[DXGI_FORMAT_BC6H_SF16]				= {"", { e_format::sfloat,		e_spec_format::bc6h }};
			m_formats[DXGI_FORMAT_BC7_TYPELESS]				= {"", { e_format::typeless,		e_spec_format::bc7 }};
			m_formats[DXGI_FORMAT_BC7_UNORM]				= {"", { e_format::unorm,		e_spec_format::bc7 }};
			m_formats[DXGI_FORMAT_BC7_UNORM_SRGB]			= {"", { e_format::unorm_srgb,	e_spec_format::bc7 }};
			m_formats[DXGI_FORMAT_B5G6R5_UNORM]				= {"", { e_format::unorm,		{_b,_5}, {_g,_6}, {_r,_5} }};
			m_formats[DXGI_FORMAT_B5G5R5A1_UNORM]			= {"", { e_format::unorm,		{_b,_5}, {_g,_5}, {_r,_5}, {_a,_1} }};
			m_formats[DXGI_FORMAT_B8G8R8A8_UNORM]			= {"", { e_format::unorm,		{_b,_8}, {_g,_8}, {_r,_8}, {_a,_8} }};
			m_formats[DXGI_FORMAT_B8G8R8X8_UNORM]			= {"", { e_format::unorm,		{_b,_8}, {_g,_8}, {_r,_8}, {_x,_8} }};
			m_formats[DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM] = {"",{ e_format::xr_bias,	{_r,_10},{_g,_10},{_b,_10},{_a,_2,unorm}}};
			m_formats[DXGI_FORMAT_B8G8R8A8_TYPELESS]		= {"", { e_format::typeless,		{_b,_8}, {_g,_8}, {_r,_8}, {_a,_8} }};
			m_formats[DXGI_FORMAT_B8G8R8A8_UNORM_SRGB]		= {"", { e_format::unorm_srgb,	{_b,_8}, {_g,_8}, {_r,_8}, {_a,_8} }};
			m_formats[DXGI_FORMAT_B8G8R8X8_TYPELESS]		= {"", { e_format::typeless,		{_b,_8}, {_g,_8}, {_r,_8}, {_x,_8} }};
			m_formats[DXGI_FORMAT_B8G8R8X8_UNORM_SRGB]		= {"", { e_format::unorm_srgb,	{_b,_8}, {_g,_8}, {_r,_8}, {_x,_8} }};
			m_formats[DXGI_FORMAT_B4G4R4A4_UNORM]			= {"", { e_format::unorm,		{_b,_4}, {_g,_4}, {_r,_4}, {_a,_4} }};
			m_formats[DXGI_FORMAT_A4B4G4R4_UNORM]			= {"", { e_format::unorm,		{_a,_4}, {_b,_4}, {_g,_4}, {_r,_4} }};
			m_formats[DXGI_FORMAT_AYUV]						= {"", { e_spec_format::AYUV }};
			m_formats[DXGI_FORMAT_Y410]						= {"", { e_spec_format::Y410 }};
			m_formats[DXGI_FORMAT_Y416]						= {"", { e_spec_format::Y416 }};
			m_formats[DXGI_FORMAT_NV12]						= {"", { e_spec_format::NV12 }};
			m_formats[DXGI_FORMAT_P010]						= {"", { e_spec_format::P010 }};
			m_formats[DXGI_FORMAT_P016]						= {"", { e_spec_format::P016 }};
			m_formats[DXGI_FORMAT_420_OPAQUE]				= {"", { e_spec_format::OP420 }};
			m_formats[DXGI_FORMAT_YUY2]						= {"", { e_spec_format::YUY2 }};
			m_formats[DXGI_FORMAT_Y210]						= {"", { e_spec_format::Y210 }};
			m_formats[DXGI_FORMAT_Y216]						= {"", { e_spec_format::Y216 }};
			m_formats[DXGI_FORMAT_NV11]						= {"", { e_spec_format::NV11 }};
			m_formats[DXGI_FORMAT_AI44]						= {"", { e_spec_format::AI44 }};
			m_formats[DXGI_FORMAT_IA44]						= {"", { e_spec_format::IA44 }};
			m_formats[DXGI_FORMAT_P8]						= {"", { e_spec_format::P8 }};
			m_formats[DXGI_FORMAT_A8P8]						= {"", { e_spec_format::A8P8 }};
			m_formats[DXGI_FORMAT_P208]						= {"", { e_spec_format::P208 }};
			m_formats[DXGI_FORMAT_V208]						= {"", { e_spec_format::V208 }};
			m_formats[DXGI_FORMAT_V408]						= {"", { e_spec_format::V408 }};
			m_formats[DXGI_FORMAT_SAMPLER_FEEDBACK_MIN_MIP_OPAQUE] = {"", { e_spec_format::sampler_feedback_minmip_opaque}};
			m_formats[DXGI_FORMAT_SAMPLER_FEEDBACK_MIP_REGION_USED_OPAQUE] = {"", { e_spec_format::sampler_feedback_mip_region_used_opaque}};
		}
	}
}
#endif // INFLUX_RHI_D3D12