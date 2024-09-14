#pragma once

#if _DLL
#define INFLUX_RG_API __declspec(dllexport)
#else
#define INFLUX_RG_API __declspec(dllimport)
#endif

#include "core/string.h"
#include "core/container/vector.h"
#include "core/container/map.h"

#include "influx_graphics/resource.h"
#include "influx_graphics/descriptorheap.h"

#include "rgcommon.h"
#include "rgpass.h"

namespace influx::graphics
{
	class device;
	class command_list;
}

namespace influx::rendergraph
{
	class rgbuffer;
	class rgtexture;
	class rgpool;
	class rglayer;
	class gpu_view_manager;

	struct texture_desc final
	{
		texture_desc() = default;
		texture_desc(uint32 w, uint32 h)
			: m_width{ w }, m_heigth{ h } {}

		uint32 m_width = 1u;
		uint32 m_heigth = 1u;
		uint32 m_depth = 1u;
		uint32 m_array_size = 1u;
		uint32 m_num_mips = 1u;
		uint32 m_sample_count = 1u;
	};

	struct buffer_desc final
	{
		size_t m_bytesize;
		size_t m_bytestride;
		graphics::e_resource_flags m_flags;
		graphics::e_resource_state m_init_state;
		graphics::e_format m_format;
	};

	class rgbuilder final
	{
	public:

	};

	class rendergraph final
	{
	public:
		INFLUX_RG_API rendergraph(graphics::device* device);

		INFLUX_RG_API void build();

		// single threaded, single command list...
		INFLUX_RG_API void execute(graphics::command_list* commandlist);

		// adding nodes & resources
		template <typename trgpass, typename... _args> requires std::is_constructible_v<trgpass, _args...>
		trgpass* add_pass(_args&&... args)
		{
			m_passes.emplace_back(new trgpass(std::forward<_args>(args)...));

			// setup the base
			rgpass* base = m_passes.back();
			rgpass_id new_id = m_passes.size() - 1u;
			base->set_id(new_id);

			// register in map
			m_id_to_pass_map[new_id] = base;

			// return the spec
			return dynamic_cast<trgpass*>(base);
		}

		INFLUX_RG_API rgtexture_id add_texture(const rgname& name, const texture_desc& desc);
		INFLUX_RG_API rgbuffer_id add_buffer(const rgname& name, const buffer_desc& desc);

		INFLUX_RG_API bool is_texture_declared(rgtexture_id id) const;
		INFLUX_RG_API bool is_buffer_declared(rgbuffer_id id) const;
		INFLUX_RG_API bool is_pass_declared(rgpass_id id) const;

		INFLUX_RG_API rgtexture* get_texture(rgtexture_id id);
		INFLUX_RG_API rgbuffer* get_buffer(rgbuffer_id id);
		INFLUX_RG_API rgpass* get_pass(rgpass_id id);

	private:
		vector<rgpass*> m_passes{};
		vector<rgbuffer*> m_buffers{};
		vector<rgtexture*> m_textures{};
		vector<rglayer*> m_layers{};

		rgpool* m_pool = nullptr;
		gpu_view_manager* m_view_manager = nullptr;

		vector<vector<uint64>> m_adjacency_lists{};
		graphics::device* m_device;

		umap<rgtexture_id, rgtexture*> m_id_to_texture_map;
		umap<rgbuffer_id, rgbuffer*> m_id_to_buffer_map;
		umap<rgpass_id, rgpass*> m_id_to_pass_map;

		enum class e_descriptor_type : uint8
		{
			readwrite,
			readonly,
			rendertarget,
			depthstencil,
			count
		};
		static constexpr uint8_t k_num_descriptor_types = static_cast<uint8_t>(e_descriptor_type::count);

		umap<rgtexture_id, graphics::descriptor_handle[k_num_descriptor_types]> m_texture_to_descriptors_map;
		umap<rgtexture_id, graphics::descriptor_handle[k_num_descriptor_types]> m_buffer_to_descriptors_map;

	private:
		void build_adjacency();
		void build_layers();

		graphics::descriptor_handle get_rtv(rgtexture_id id);
		graphics::descriptor_handle get_dsv(rgtexture_id id);
		graphics::descriptor_handle get_readonly(rgtexture_id id);
		graphics::descriptor_handle get_readwrite(rgtexture_id id);
	};
}