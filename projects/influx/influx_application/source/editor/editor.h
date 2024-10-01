#pragma once
#include "core/container/list.h"
#include "core/scene/camera.h"

struct ImDrawData;

namespace influx::application
{
	class editor final
	{
	public:
		editor();
		void set_window_dimensions(const math::vectorf2& dimensions);
		void update();
		ImDrawData* get_imgui_drawdata();

		using imgui_callback = function<void()>;
		void subscribe(const imgui_callback& callback);
		void unsubscribe(const imgui_callback& callback);

		static void draw_transform(const math::transform3D& transform, const string& tag = "");
		static void draw_vector3(const math::vectorf3& vec3, const string& tag = "");
		static void draw_mat4x4(const math::matrix4x4f& mat4x4, const string& tag = "");
		static void draw_camera(const influx::scene::camera& camera, const string& tag = "");

	private:
		list<imgui_callback> m_callbacks{};

		void draw_imgui();
	};
}