#pragma once

#if _DLL
#define INFLUX_FILE_API __declspec(dllexport)
#else
#define INFLUX_FILE_API __declspec(dllimport)
#endif


// influx::core
#include "core/basetypes.h"
#include "core/file.h"
#include <fstream>

namespace influx::files
{
	namespace detail
	{
		class file_interface
		{
		public:
			void INFLUX_FILE_API save(const file& file);
			void INFLUX_FILE_API load(const file& file);
			bool INFLUX_FILE_API is_loading() const;

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

	class gamefile final : public detail::file_interface
	{
		INFLUX_FILE_API virtual bool serialize() override;

	public:
		uint32 m_id;
		string m_name;
	};

	class actorfile final : public detail::file_interface
	{

	};

	class componentfile final : public detail::file_interface
	{

	};

	class scenefile final : public detail::file_interface
	{
		INFLUX_FILE_API virtual bool serialize() override;

	public:
		// actors in scene
		INFLUX_FILE_API
			void add_actor(const uint32 id, const string& name);
		
		INFLUX_FILE_API
			uint32 get_num_actors() const;

		// components in scene
		vector<uint32> m_actor_ids;
		vector<string> m_actor_names;
		vector<uint32> m_actor_component_types;
	};

	class texturefile final : public detail::file_interface
	{
		INFLUX_FILE_API virtual bool serialize() override;
	};
}