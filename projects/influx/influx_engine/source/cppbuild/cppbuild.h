#pragma once

// influx::core
#include "core/file.h"

// influx::script
#include "influx_script.h"

namespace influx::engine
{
	class cpp_build final
	{
	public:
		enum class e_state
		{
			idle,
			building,
			count
		};

		void set_target_folder(const file& folder)
		{
			m_target_folder = folder;
		}

		void build()
		{
			if (is_idle())
			{
				const string root_folder = m_target_folder.m_path_full;
				const string include_folder = root_folder + "/include/";
				const string source_folder = root_folder + "/source/";


			}
		}

		bool is_idle() const
		{
			return m_state == e_state::idle;
		}

	private:
		file m_target_folder;
		e_state m_state;
	};
}