#include "engine_pch.h"
#include "engine_files.h"

// cereal library
#include "influx_cereal_layer.h"

namespace influx::engine
{
	namespace detail
	{
		void file_interface::save(const file& file)
		{
			m_is_loading = false;

			create_file(file.m_path_full);

			m_ofstream.close();
			m_ofstream.open(file.m_path_full);
			influx_assert(m_ofstream.is_open());

			m_name = file.m_filename;
			
			serialize();

			m_file = file;
		}

		void file_interface::load(const file& file)
		{
			m_is_loading = true;

			m_ifstream.close();
			m_ifstream.open(file.m_path_full);
			influx_assert(m_ifstream.is_open());

			m_name = file.m_filename;

			serialize();

			m_file = file;
		}

		bool file_interface::is_loading() const
		{
			return m_is_loading;
		}

		std::ofstream& file_interface::get_ofs()
		{
			return m_ofstream;
		}

		std::ifstream& file_interface::get_ifs()
		{
			return m_ifstream;
		}
	}

#define archive_json(variable) \
	if (is_loading()) { cereal::JSONInputArchive AR(get_ifs()); AR(variable); } \
	else { cereal::JSONOutputArchive AR(get_ofs()); AR(variable); }
#define archive_bin(variable) \
	if (is_loading()) { cereal::BinaryInputArchive AR(get_ifs()); AR(variable); } \
	else { cereal::BinaryOutputArchive AR(get_ofs()); AR(variable); }

#define archive(variable) archive_json(variable)

	bool file_game::serialize()
	{
		archive(m_id);
		return true;
	}
}

