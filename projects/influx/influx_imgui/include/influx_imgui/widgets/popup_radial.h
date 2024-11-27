#pragma once

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
	class popup_radial final
	{
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

		void set_items(const vector<const char*>& items);
		void add_item(const char*);
		void remove_item(const char*);
		void remove_item(uint32 at_index);
		const char* get_item(uint32 at_index) const;
		uint32 get_item_index(const char*) const;

		void set_id(const char* id);
		const char* get_id() const;

		bool has_selection() const;
		const char* get_selected();

	private:
		bool m_is_visible = false;
		math::vectorf2 m_position = {};
		const char* m_id = "##piepopup";
		vector<const char*> m_items = {};
		int m_selected = -1;
		float m_radius = 10.0f;
	};

#pragma region impl
	inline popup_radial::popup_radial(const math::vectorf2& spawn_position)
		: m_position{ spawn_position }
		, m_is_visible{ true }
		, m_items{}
		, m_selected{ -1 }
		, m_radius{}
	{
		set_position(spawn_position);
	}

	inline popup_radial::~popup_radial()
	{

	}

	inline void popup_radial::render(const math::vectorf2& mouse_pos)
	{
		m_selected = PiePopupSelectMenu(
			translate(m_position),
			translate(mouse_pos),
			get_radius(),
			get_id(),
			m_items.data(),
			(int)m_items.size(),
			&m_selected,
			!m_is_visible);

		// this opens/shows the popup
		if (m_is_visible)
			ImGui::OpenPopup(m_id);
	}

	inline void popup_radial::set_position(const math::vectorf2& position)
	{
		m_position = position;
	}

	inline const math::vectorf2& popup_radial::get_position() const
	{
		return m_position;
	}

	inline void popup_radial::set_visible(bool new_vis)
	{
		m_is_visible = new_vis;
	}

	inline bool popup_radial::is_visible() const
	{
		return m_is_visible;
	}

	inline void popup_radial::set_radius(float new_radius)
	{
		m_radius = new_radius;
	}

	inline float popup_radial::get_radius() const
	{
		return m_radius;
	}

	inline void popup_radial::set_items(const vector<const char*>& items)
	{
		m_items = items;
	}

	inline void popup_radial::add_item(const char* new_item)
	{
		m_items.push_back(new_item);
	}

	inline const char* popup_radial::get_item(uint32 at_index) const
	{
		if (at_index < m_items.size())
		{
			return m_items[at_index];
		}
		return "";
	}

	inline void popup_radial::remove_item(const char* item)
	{
		const uint32 item_idx = get_item_index(item);
		remove_item(item_idx);
	}

	inline void popup_radial::remove_item(uint32 at_index)
	{
		if (at_index < m_items.size())
		{
			m_items[at_index] = m_items.back();
			m_items.pop_back();
		}
	}

	inline uint32 popup_radial::get_item_index(const char* item) const
	{
		for (uint64 i = 0u; i < m_items.size(); ++i)
		{
			const string str = m_items[i];
			if (str.compare(item))
			{
				return (uint32)i;
			}
		}

		return (uint32)-1;
	}

	inline void popup_radial::set_id(const char* id)
	{
		m_id = id;
	}

	inline const char* popup_radial::get_id() const
	{
		return m_id;
	}

	inline bool popup_radial::has_selection() const
	{
		return m_selected >= 0u && m_selected < m_items.size();
	}

	inline const char* popup_radial::get_selected()
	{
		if (has_selection())
		{
			return m_items[m_selected];
		}

		return "";
	}
#pragma endregion
}