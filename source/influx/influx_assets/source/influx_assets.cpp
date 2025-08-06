#include "influx_assets.h"

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

	void serializeable::save(const path& file)
	{
		m_is_loading = false;

		// pre-create
		path::create(file.m_path_full);

		m_ofstream.close();
		m_ofstream.open(file.m_path_full);
		influx_assert(m_ofstream.is_open());

		m_name = file.m_filename;
		m_file = file;

		on_serialize();
	}

	void serializeable::load(const path& file)
	{
		m_is_loading = true;

		m_ifstream.close();
		m_ifstream.open(file.m_path_full);
		influx_assert(m_ifstream.is_open());

		m_file = file;
		m_name = file.m_filename;

		on_serialize();
	}

	const path& serializeable::get_file() const
	{
		return m_file;
	}

	const string& serializeable::get_filename() const
	{
		return get_file().m_filename;
	}

	bool serializeable::has_file() const
	{
		return path::exists(m_file);
	}

	const string& gameproject::get_name() const
	{
		return get_file().m_filename;
	}
}