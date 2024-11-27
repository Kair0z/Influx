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
#include "influx_engine/engine/layer.h"

// ImGui
struct ImGuiContext;
#pragma endregion

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

		template <typename _layer>
		_layer* create_layer();

		// inheritable interface
		struct ctx_update final
		{
			frame_time m_frametime;

			ctx_update() = default;
			ctx_update(const frame_time& frtime);
		};

		virtual void on_config(app_config&, game_config&);
		virtual void on_start();
		virtual void on_update(const ctx_update& ctx);
		virtual void on_cleanup();

		const game_config& get_config() const;

		// don't touch this
		void update(const ctx_update&);

	private:
		game_config m_config;
		layer m_root_layer{};
	};

	class INFLUX_ENGINE_API editor_module : public base_module
	{
	public:
		virtual void on_config(app_config&, editor_config&);
		virtual void on_imgui(ImGuiContext& ctx);
		virtual void on_cleanup();

		virtual ~editor_module() = default;
	};

#pragma region impl
	template<typename _layer>
	inline _layer* game_module::create_layer()
	{
		_layer* new_layer = new _layer();
		m_root_layer.add_child(new_layer);
		return new_layer;
	}
#pragma endregion
}

namespace influx::engine::detail
{
#define influx_engine_game(x) \
	influx::engine::base_module* create_module() { return new x(); } \

#define influx_engine_editor(x) \
	influx::engine::base_module* create_module() { return new x(); } \

	bool INFLUX_ENGINE_API run_engine(base_module* mod);
}

