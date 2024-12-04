#pragma once

// influx::core
#include "core/file.h"
#include "core/string.h"
#include "core/container/vector.h"

// influx::platform
#include "influx_platform/platform.h"

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
		binaries,
		count
	};

	static file get_engine_directory(engine_directory directory)
	{
		// temp: HARDCODED builds are ran in /influx/bin/[config]/influx_game/
		const string& root = platform::platform::get_current_directory() + "/../../../";
		switch (directory)
		{
		case engine_directory::root:			return root;
		case engine_directory::assets:		return root + "/assets/";
		case engine_directory::staged:		return root + "/staged/";
		case engine_directory::binaries:		return root + "/bin/";
		case engine_directory::intermediate: return root + "/int/";
		case engine_directory::games:		return root + "/games/";
		}
		return {};
	}

	static file get_game_directory(const string& game_name, game_directory directory)
	{
		const file& games_directory = get_engine_directory(engine_directory::games);
		const file game_directory = games_directory.m_path_full + "/" + game_name + "/";
		influx_assert(game_directory.is_directory());

		switch (directory)
		{
		case game_directory::root: return game_directory;
		case game_directory::assets: return game_directory.m_path_full + "/assets/";
		case game_directory::binaries: return game_directory.m_path_full + "/binaries/";
		}
		return {};
	}

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