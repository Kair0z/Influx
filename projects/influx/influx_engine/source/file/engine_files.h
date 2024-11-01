#pragma once

// influx::core
#include "core/file.h"
#include "core/string.h"

// STL
#include <fstream>

namespace influx::engine
{
	namespace detail
	{
		class file_interface
		{
		public:
			void save(const file& file);
			void load(const file& file);
			bool is_loading() const;

		protected:
			std::ofstream& get_ofs();
			std::ifstream& get_ifs();
			uint32 m_id;

		private:
			virtual bool serialize() = 0;

			file m_file = {};
			string m_name = {};
			bool m_is_loading = false;
			std::ofstream m_ofstream{};
			std::ifstream m_ifstream{};
		};
	}

	class file_game : public detail::file_interface
	{
		bool serialize() override;

	public:
		uint32 m_id;
		string m_name;
	};

	class file_scene
	{

	};

	class file_texture
	{

	};
}