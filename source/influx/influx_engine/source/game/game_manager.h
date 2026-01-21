#pragma once

// influx::core
#include "core/container/vector.h"

// influx::engine
#include "world/entity.h"
#include "influx_engine/engine_api.h"

// influx::platform
#include "influx_platform/library.h"

namespace influx::engine
{
	class game_library final
	{
		string m_filepath;
		platform::library* m_library = nullptr;

	public:
		game_library() = default;
		game_library(const string& filepath)
		{
			load(filepath).get();
		}

		bool is_loaded() const
		{
			return m_library != nullptr;
		}

		result<> load(const string& filepath, bool reload = false)
		{
			if (is_loaded() && reload)
			{
				delete m_library;
			}
			if (!is_loaded() || reload)
			{
				m_filepath = filepath;
				m_library = platform::library::load(filepath);
				if (m_library == nullptr)
					return result<>::make_error("failed loading library at path!");
			}

			return {};
		}

		result<> call_engine_init()
		{
			if (!is_loaded())
				return result<>::make_error("library failed loading!");

			void* func = m_library->get_func_address("engine_init");
			if (func)
			{
				static game_api api{};
				api.log = engine::log;

				typedef void (*engine_init_func)(game_api*);
				engine_init_func engine_init = (engine_init_func)func;
				engine_init(&api);
				return {};
			}
			
			return result<>::make_error("library function engine_init() not found!");
		}

		result<> call_start()
		{
			if (!is_loaded())
				return result<>::make_error("library failed loading!");

			m_library->call("start");
			return {};
		}
		result<> call_tick()
		{
			if (!is_loaded())
				return result<>::make_error("library failed loading!");

			m_library->call("tick");
			return {};
		}
		result<> call_end()
		{
			if (!is_loaded())
				return result<>::make_error("library failed loading!");

			m_library->call("end");
			return {};
		}
	};

	class game_manager final
	{
		enum class state
		{
			idle,
			running
		};

	public:
		game_manager();

		void start();
		void tick();
		void end();

		entity create_entity();
		~game_manager();
		
	private:
		state m_state = state::idle;
		vector<entity> m_entities = {};
		game_library m_game_library{};

		void setup_camera();
	};
}