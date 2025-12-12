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
		path m_file = {};
		string m_name = {};
		bool m_is_loading = false;
		std::wofstream m_ofstream{};
		std::wifstream m_ifstream{};

	public:
		INFLUX_ASSETS_API void save(const path& file);
		INFLUX_ASSETS_API void load(const path& file);
		INFLUX_ASSETS_API bool is_loading() const;

	protected:
		std::wofstream& get_ofs();
		std::wifstream& get_ifs();

	private:
		virtual bool serialize() = 0;
		void serialize_base();
	};

	class flx_actor final : public flx_asset
	{
		uint32 m_id;
		string m_name;

	private:
		INFLUX_ASSETS_API bool serialize() override;
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