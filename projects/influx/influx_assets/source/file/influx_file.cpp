#include "assets_pch.h"
#include "influx_assets.h"

#include "core/string.h"

#include "cereal/types/string.hpp"
#include "cereal/archives/json.hpp"
#include "cereal/archives/binary.hpp"

namespace influx::assets
{
#define archive_json(variable) \
	if (is_loading()) { cereal::JSONInputArchive AR(get_ifs()); AR(variable); } \
	else { cereal::JSONOutputArchive AR(get_ofs()); AR(variable); }

#define archive_bin(variable) \
	if (is_loading()) { cereal::BinaryInputArchive AR(get_ifs()); AR(variable); } \
	else { cereal::BinaryOutputArchive AR(get_ofs()); AR(variable); }

#define archive(variable) archive_json(variable)

#pragma region serialize_impl
	template <class _archive>
	void serialize(_archive& arch, flx_scene& scene)
	{
		arch(scene.m_id);
	}
#pragma endregion

	void flx_asset::save(const file& file)
	{
		m_is_loading = false;

		// pre-create the file 
		create_file(file.m_path_full);

		m_ofstream.close();
		m_ofstream.open(file.m_path_full);
		influx_assert(m_ofstream.is_open());

		m_name = file.m_filename;

		serialize_base();
		serialize();

		m_file = file;
	}

	void flx_asset::load(const file& file)
	{
		m_is_loading = true;
		
		m_ifstream.close();
		m_ifstream.open(file.m_path_full);
		influx_assert(m_ifstream.is_open());

		m_name = file.m_filename;

		serialize_base();
		serialize();

		m_file = file;
	}

	void flx_asset::serialize_base()
	{
		archive(m_name);
	}

	bool flx_asset::is_loading() const
	{
		return m_is_loading;
	}

	std::ifstream& flx_asset::get_ifs()
	{
		return m_ifstream;
	}

	std::ofstream& flx_asset::get_ofs()
	{
		return m_ofstream;
	}

	bool flx_scene::serialize()
	{
		archive(*this);
		return true;
	}
}