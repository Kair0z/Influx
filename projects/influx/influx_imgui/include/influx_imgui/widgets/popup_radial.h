#pragma once

// influx::core
#include "core/container/map.h"

// imgui
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

namespace influx::imgui
{
	// https://github.com/ocornut/imgui/issues/434
	// Return >= 0 on mouse release
	// Optional int* p_selected display and update a currently selected item
	inline int PiePopupSelectMenu(
		const ImVec2& center,
		const ImVec2& mousepos,
		const float radius,
		const char* popup_id,
		const char** items,
		int items_count,
		int* p_selected,
		bool is_release)
	{
		int ret = -1;

		// FIXME: Missing a call to query if Popup is open so we can move the PushStyleColor inside the BeginPopupBlock (e.g. IsPopupOpen() in imgui.cpp)
		// FIXME: Our PathFill function only handle convex polygons, so we can't have items spanning an arc too large else inner concave edge artifact is too visible, hence the ImMax(7,items_count)
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
		ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
		if (ImGui::BeginPopup(popup_id))
		{
			const ImVec2 drag_delta = ImVec2(mousepos.x - center.x, mousepos.y - center.y);
			const float drag_dist2 = drag_delta.x * drag_delta.x + drag_delta.y * drag_delta.y;

			const ImGuiStyle& style = ImGui::GetStyle();
			const float RADIUS_MAX = radius;
			const float RADIUS_MIN = RADIUS_MAX * 0.25f;
			const float RADIUS_INTERACT_MIN = 20.0f;
			const int ITEMS_MIN = 6;

			ImDrawList* draw_list = ImGui::GetWindowDrawList();
			//ImGuiWindow* window = ImGui::GetCurrentWindow();
			draw_list->PushClipRectFullScreen();
			draw_list->PathArcTo(center, (RADIUS_MIN + RADIUS_MAX) * 0.5f, 0.0f, IM_PI * 2.0f * 0.99f, 32);   // FIXME: 0.99f look like full arc with closed thick stroke has a bug now
			draw_list->PathStroke(ImColor(0, 0, 0), true, RADIUS_MAX - RADIUS_MIN);

			const float item_arc_span = 2 * IM_PI / ImMax(ITEMS_MIN, items_count);
			float drag_angle = atan2f(drag_delta.y, drag_delta.x);
			if (drag_angle < -0.5f * item_arc_span)
				drag_angle += 2.0f * IM_PI;
			//ImGui::Text("%f", drag_angle);    // [Debug]

			int item_hovered = -1;
			for (int item_n = 0; item_n < items_count; item_n++)
			{
				const char* item_label = items[item_n];
				const float item_ang_min = item_arc_span * (item_n + 0.02f) - item_arc_span * 0.5f; // FIXME: Could calculate padding angle based on how many pixels they'll take
				const float item_ang_max = item_arc_span * (item_n + 0.98f) - item_arc_span * 0.5f;

				bool hovered = false;
				if (drag_dist2 >= RADIUS_INTERACT_MIN * RADIUS_INTERACT_MIN)
				{
					if (drag_angle >= item_ang_min && drag_angle < item_ang_max)
						hovered = true;
				}
				bool selected = p_selected && (*p_selected == item_n);

				int arc_segments = (int)(32 * item_arc_span / (2 * IM_PI)) + 1;
				draw_list->PathArcTo(center, RADIUS_MAX - style.ItemInnerSpacing.x, item_ang_min, item_ang_max, arc_segments);
				draw_list->PathArcTo(center, RADIUS_MIN + style.ItemInnerSpacing.x, item_ang_max, item_ang_min, arc_segments);
				//draw_list->PathFill(window->Color(hovered ? ImGuiCol_HeaderHovered : ImGuiCol_FrameBg));
				draw_list->PathFillConvex(hovered ? ImColor(100, 100, 150) : selected ? ImColor(120, 120, 140) : ImColor(70, 70, 70));

				ImVec2 text_size = ImGui::GetFont()->CalcTextSizeA(ImGui::GetFontSize(), FLT_MAX, 0.0f, item_label);
				ImVec2 text_pos = ImVec2(
					center.x + cosf((item_ang_min + item_ang_max) * 0.5f) * (RADIUS_MIN + RADIUS_MAX) * 0.5f - text_size.x * 0.5f,
					center.y + sinf((item_ang_min + item_ang_max) * 0.5f) * (RADIUS_MIN + RADIUS_MAX) * 0.5f - text_size.y * 0.5f);
				draw_list->AddText(text_pos, ImColor(255, 255, 255), item_label);

				if (hovered)
					item_hovered = item_n;
			}
			draw_list->PopClipRect();

			if (is_release)
			{
				ImGui::CloseCurrentPopup();
				ret = item_hovered;
				if (p_selected)
					*p_selected = item_hovered;
			}

			ImGui::EndPopup();
		}
		ImGui::PopStyleColor(2);
		return ret;
	}

	// helper class which uses the PiePopupSelect, and manages its own state

	template <typename _t, uint32 _c = 6u>
	class popup_radial final
	{
		using item_type = _t;
		static constexpr uint32 capacity = _c;

	public:
		popup_radial() = default;
		popup_radial(const math::vectorf2& spawn_position);
		~popup_radial();

		void render(const math::vectorf2& mouse_pos);

		void set_position(const math::vectorf2&);
		const math::vectorf2& get_position() const;

		void set_visible(bool);
		bool is_visible() const;

		void set_radius(float);
		float get_radius() const;

		void set_item(const string& name, const item_type& item);
		void remove_item(const string& name);
		item_type* get_item(const string& name) const;

		void set_id(const char* id);
		const char* get_id() const;

		bool has_selection() const;
		_t* get_selected();

	private:
		bool m_is_visible = false;
		math::vectorf2 m_position = {};
		const char* m_id = "##piepopup";

		umap<string, item_type> m_items{};

		string m_selected = "";
		float m_radius = 10.0f;
	};

#pragma region impl
	template <typename _t, uint32 _c>
	inline popup_radial<_t, _c>::popup_radial(const math::vectorf2& spawn_position)
		: m_position{ spawn_position }
		, m_is_visible{ true }
		, m_items{}
		, m_selected{ ""}
		, m_radius{}
	{
		set_position(spawn_position);
	}

	template <typename _t, uint32 _c>
	inline popup_radial<_t, _c>::~popup_radial()
	{

	}

	template <typename _t, uint32 _c>
	inline void popup_radial<_t, _c>::render(const math::vectorf2& mouse_pos)
	{
		vector<const char*> item_titles{};
		for (const auto& pair : m_items)
		{
			item_titles.push_back(pair.first.c_str());
		}

		int selected_idx = PiePopupSelectMenu(
			translate(m_position),
			translate(mouse_pos),
			get_radius(),
			get_id(),
			item_titles.data(),
			(int)item_titles.size(),
			&selected_idx,
			!m_is_visible);

		if (selected_idx >= 0)
		{
			m_selected = item_titles[selected_idx];
		}
		else
		{
			m_selected = "";
		}

		// this opens/shows the popup
		if (m_is_visible)
			ImGui::OpenPopup(m_id);
	}

	template <typename _t, uint32 _c>
	inline void popup_radial<_t, _c>::set_position(const math::vectorf2& position)
	{
		m_position = position;
	}

	template <typename _t, uint32 _c>
	inline const math::vectorf2& popup_radial<_t, _c>::get_position() const
	{
		return m_position;
	}

	template <typename _t, uint32 _c>
	inline void popup_radial<_t, _c>::set_visible(bool new_vis)
	{
		m_is_visible = new_vis;
	}

	template <typename _t, uint32 _c>
	inline bool popup_radial<_t, _c>::is_visible() const
	{
		return m_is_visible;
	}

	template <typename _t, uint32 _c>
	inline void popup_radial<_t, _c>::set_radius(float new_radius)
	{
		m_radius = new_radius;
	}

	template <typename _t, uint32 _c>
	inline float popup_radial<_t, _c>::get_radius() const
	{
		return m_radius;
	}

	template <typename _t, uint32 _c>
	inline void popup_radial<_t, _c>::set_item(const string& title, const _t& item)
	{
		m_items[title] = item;
	}

	template <typename _t, uint32 _c>
	inline _t* popup_radial<_t, _c>::get_item(const string& title) const
	{
		if (m_items.contains(title))
		{
			return m_items[title];
		}

		return nullptr;
	}

	template <typename _t, uint32 _c>
	inline void popup_radial<_t, _c>::remove_item(const string& name)
	{
		if (m_items.contains(name))
		{
			m_items.remove(name);
		}
	}

	template <typename _t, uint32 _c>
	inline void popup_radial<_t, _c>::set_id(const char* id)
	{
		m_id = id;
	}

	template <typename _t, uint32 _c>
	inline const char* popup_radial<_t, _c>::get_id() const
	{
		return m_id;
	}

	template <typename _t, uint32 _c>
	inline bool popup_radial<_t, _c>::has_selection() const
	{
		return (m_selected != "" && m_items.contains(m_selected));
	}

	template <typename _t, uint32 _c>
	inline _t* popup_radial<_t, _c>::get_selected()
	{
		if (has_selection())
		{
			return &m_items.at(m_selected);
		}

		return nullptr;
	}
#pragma endregion
}