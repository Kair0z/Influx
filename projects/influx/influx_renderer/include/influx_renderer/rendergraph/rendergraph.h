#pragma once

#include "core/string.h"
#include "core/container/vector.h"

#include "rgpass.h"

namespace influx::graphics
{
	
}

namespace influx::renderer
{
	class texture;
	struct texture_desc;
}

namespace influx::renderer
{
	class rendergraph;

	class rgchild
	{
	public:
#if _DEBUG
		void set_name(const string& name)
		{
			m_name = name;
		}

		const string& get_name() const
		{
			return m_name;
		}
#endif

	protected:
		rgchild() = default;

	private:
#if _DEBUG
		string m_name{};
#endif
	};

	class rgtexture final : public rgchild
	{
	private:
		rgtexture() = default;

	public:
	};

	class rgbuffer final : public rgchild
	{
	private:
		rgbuffer() = default;

	public:
	};

	class rgbuilder final
	{
	public:
		// find external resources
		void find_buffer();
		void find_texture();

		// register external resources
		void register_buffer();
		void register_texture();

		// create tracked srv
		void create_srv_buffer();
		void create_srv_texture();

		// create tracked uav
		void create_uav_buffer();
		void create_uav_texture();

		// add pass node
		template <typename _parameters, typename _func>
		rgpass_base* add_pass(const _parameters* params, _func&& func)
		{

		}

		// link 2 passes together
		void add_dependency(rgpass_base* producer, rgpass_base* consumer)
		{

		}

	private:
		vector<rgpass_base*> m_passes;
		vector<rgtexture*> m_textures;
		vector<rgbuffer*> m_buffers;
	};

	class rglayer final
	{
	private:
		friend class rendergraph;
		rglayer(rendergraph& rg) : m_rg{ rg } {}

		void execute();

		rendergraph& m_rg;
		vector<rgpass_base*> m_passes;
	};

	// resource pool
	class rgpool final
	{
		friend class rendergraph;
		rgpool() = default;

	private:
		void tick();

		texture* allocate_texture(const texture_desc& args);
		bool release_texture(texture* tex);


	};

	class rendergraph final
	{
	public:
		void execute();

	private:
		vector<rglayer*> m_layers{};
	};
}