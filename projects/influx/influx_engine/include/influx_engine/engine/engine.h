#pragma once

#if _DLL
#define INFLUX_ENGINE_API __declspec(dllexport)
#else
#define INFLUX_ENGINE_API __declspec(dllimport)
#endif

#pragma region dependencies
// influx::core
#include "core/string.h"
#include "core/math/vector.h"
#include "core/file.h"
#include "core/result.h"

// influx::engine
#include "influx_engine/game/game.h"
#include "influx_engine/editor/editor.h"
#include "influx_engine/engine/config.h"
#include "influx_engine/engine/common.h"
#include "influx_engine/engine/component.h"
#include "influx_engine/engine/gameobject.h"
#include "influx_engine/engine/layergraph.h"

namespace influx::engine
{
	class world;
}

// ImGui
struct ImGuiContext;
#pragma endregion

// module interface
namespace influx::engine
{
	class INFLUX_ENGINE_API base_module
	{
	public:
		virtual ~base_module() = default;

	protected:
		base_module() = default;
	};

	class INFLUX_ENGINE_API game_module : public base_module
	{
	public:
		virtual ~game_module() = default;

		// -- layergraph
		template <class _ltype, class ..._args>
		_ltype* create_rootlayer(_args&&... args)
		{
			return m_layergraph.create_layer<_ltype, _args...>(nullptr, args...);
		}

		// -- deriveable interface
		virtual void on_config(app_config&, game_config&);
		virtual void on_start();
		virtual void on_update(const update_context& ctx);
		virtual void on_cleanup();

		const game_config& get_config() const;

		// don't touch this
		void update(const update_context&);

	private:
		game_config m_config{};
		layergraph m_layergraph{};
	};

	class INFLUX_ENGINE_API editor_module : public base_module
	{
	public:
		virtual void on_config(app_config&, editor_config&);
		virtual void on_imgui(ImGuiContext& ctx);
		virtual void on_cleanup();

		virtual ~editor_module() = default;
	};
}

namespace influx::engine::detail
{
#define influx_engine_game(x) \
	influx::engine::base_module* create_module() { return new x(); } \

#define influx_engine_editor(x) \
	influx::engine::base_module* create_module() { return new x(); } \

	INFLUX_ENGINE_API bool run_engine(base_module* mod);
}

