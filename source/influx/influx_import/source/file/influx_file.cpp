#include "import_pch.h"
#include "influx_import.h"

#include "cereal/types/string.hpp"
#include "cereal/archives/json.hpp"
#include "cereal/archives/binary.hpp"

// these custom serialization functions need to be in the cereal namespace for cereal to access them!
namespace cereal
{
	// influx::math::matrix4x4f
	template <class _archive> void serialize(_archive& arch, influx::math::matrix4x4f& matrix)
	{
		arch(matrix.m_data);
	}

	// influx::imp::flx_scene::node
	template <class _archive> void serialize(_archive& arch, influx::imp::flx_scene::node& node)
	{
		arch(node.m_id);
		arch(node.m_transform);
	}

	// influx::imp::flx_scene
	template <class _archive> void serialize(_archive& arch, influx::imp::flx_scene& scene)
	{
		arch(scene.m_id);
		scene.m_hierarchy.traverse([&arch](influx::flat_tree<influx::imp::flx_scene::node>::node& node)
		{
			arch(node.m_data);
		});
	}
}


namespace influx::imp
{
#define archive_json(variable) \
	if (is_loading()) { cereal::JSONInputArchive AR(get_ifs()); AR(variable); } \
	else { cereal::JSONOutputArchive AR(get_ofs()); AR(variable); }

#define archive_bin(variable) \
	if (is_loading()) { cereal::BinaryInputArchive AR(get_ifs()); AR(variable); } \
	else { cereal::BinaryOutputArchive AR(get_ofs()); AR(variable); }

#define archive(variable) archive_json(variable)

	void flx_asset::save(const path& file)
	{
		m_is_loading = false;

		// pre-create the file 
		path::create_file(file.get_full_path());

		m_ofstream.close();
		m_ofstream.open(file.get_full_path());
		influx_assert(m_ofstream.is_open());

		const bool without_extension = false;
		m_name = to_string(file.get_filename(without_extension));

		serialize_base();
		serialize();

		m_file = file;
	}

	void flx_asset::load(const path& file)
	{
		m_is_loading = true;
		
		m_ifstream.close();
		m_ifstream.open(file.get_full_path());
		influx_assert(m_ifstream.is_open());

		const bool without_extension = false;
		m_name = to_string(file.get_filename(without_extension));

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