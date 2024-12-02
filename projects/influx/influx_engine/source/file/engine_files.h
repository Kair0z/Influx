#pragma once

// influx::core
#include "core/file.h"
#include "core/string.h"
#include "core/container/vector.h"

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

	enum class engine_directory : uint8
	{
		root,
		assets,
		staged,
		intermediate,
		binaries,
		games,
		count
	};

	enum class game_directory : uint8
	{
		root,
		assets,
		count
	};

	class file_game : public detail::file_interface
	{
		bool serialize() override;

	public:
		uint32 m_id;
		string m_name;
	};

	class file_scene : public detail::file_interface
	{
		bool serialize() override;

	public:
		void add_actor(
			const uint32 id, 
			const string& name);

		uint32 get_num_actors() const;

		vector<uint32> m_actor_ids;
		vector<string> m_actor_names;
		vector<uint32> m_actor_component_types;
	};

	class file_texture
	{

	};
}