#pragma once

#include "core/string.h"
#include "core/file.h"
#include "core/math/matrix.h"
#include "core/container/list.h"
#include "core/graph/hierarchy.h"

#include <fstream>

namespace influx::imp
{
	class flx_asset
	{
	public:
		INFLUX_ASSETS_API void save(const path& file);
		INFLUX_ASSETS_API void load(const path& file);

		INFLUX_ASSETS_API bool is_loading() const;

	protected:
		std::ofstream& get_ofs();
		std::ifstream& get_ifs();

	private:
		virtual bool serialize() = 0;
		void serialize_base();

		path m_file = {};
		string m_name = {};
		bool m_is_loading = false;
		std::ofstream m_ofstream{};
		std::ifstream m_ifstream{};
	};

	class flx_actor final : public flx_asset
	{
	private:
		INFLUX_ASSETS_API bool serialize() override;

	public:
		uint32 m_id;
		string m_name;
	};

	class flx_scene final : public flx_asset
	{
	private:
		INFLUX_ASSETS_API bool serialize() override;

	public:
		struct node final
		{
			math::matrix4x4f m_transform;
			uint32 m_id;
		};

		uint32 m_id;
		flat_tree<node> m_hierarchy;
	};
}