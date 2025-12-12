#pragma once

#ifndef IMGUI_WDIGETS
#define IMGUI_WDIGETS

// influx::core
#include "core/container/vector.h"
#include "core/string.h"
#include "core/math/transform.h"
#include "core/enum.h"

// imgui
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

// influx::imgui
#include "influx_imgui/imgui_translation.h"
#include "influx_imgui/widgets/popup_radial.h"
#include "influx_imgui/widgets/text_editor.h"
#include "influx_imgui/widgets/imfilebrowser.h"

namespace influx::imgui
{
	inline void transform3D(const string& title, const math::transform3D& transform)
	{
		ImGui::Text( title.get_std().c_str() );

		static const string kfloat_precision = "%.2f";
		static const string ktriple_float = kfloat_precision + "," + kfloat_precision + "," + kfloat_precision;

		const math::float3 position = transform.get_position();
		ImGui::Text(("position: \t" + ktriple_float).get_std().c_str(), position.x, position.y, position.z);

		const math::rotation rotation = transform.get_rotation();
		const math::float3 eulers = rotation.get_euler_angles();
		ImGui::Text(("rotation: \t" + ktriple_float).get_std().c_str(), eulers.x, eulers.y, eulers.z);

		const math::float3 scale = transform.get_scale();
		ImGui::Text(("scale: \t" + ktriple_float).get_std().c_str(), scale.x, scale.y, scale.z);
	}

	struct scoped_style_var final
	{
	public:
		explicit scoped_style_var(ImGuiStyleVar style, const float& value)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, value);
		}

		explicit scoped_style_var(ImGuiStyleVar style, const ImVec2& value)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, value);
		}

		~scoped_style_var()
		{
			ImGui::PopStyleVar();
		}
	};

	struct logger final
	{
		enum class e_flags : uint32
		{
			none        = 0,
            info        = 1 << 0,
            warning     = 1 << 1,
            error       = 1 << 2,
            all         = info | warning | error
		};

		ImGuiTextIndex m_textindex{};
		ImGuiTextBuffer m_textbuffer{};
		e_flags m_flags{};
        bool m_is_open = true;
        int m_skipped;
        bool m_is_minimal = true;

		inline void push(const char* fmt, va_list args)
		{
			const int old_size = m_textbuffer.size();
			m_textbuffer.appendf("[%05d] ", 1u);
			m_textbuffer.appendfv(fmt, args);
			m_textindex.append(m_textbuffer.c_str(), old_size, m_textbuffer.size());
		}

        inline static bool checkbox_flags(const char* name, e_flags* flags, e_flags flags_value)
        {
            bool all_on = has_all_flags(*flags, flags_value);
            bool any_on = has_any_flag(*flags, flags_value);
            bool pressed;
            if (!all_on && any_on)
            {
                ImGuiContext& g = *GImGui;
                g.NextItemData.ItemFlags |= ImGuiItemFlags_MixedValue;
                pressed = ImGui::Checkbox(name, &all_on);
            }
            else
            {
                pressed = ImGui::Checkbox(name, &all_on);

            }
            if (pressed)
            {
#if 0
                if (all_on)
                    *flags |= flags_value;
                else
                    *flags &= ~flags_value;
#endif
            }
            return pressed;
        }

        inline static void same_line_or_wrap(const ImVec2& size)
        {
            ImGuiContext& g = *GImGui;
            ImGuiWindow* window = g.CurrentWindow;
            ImVec2 pos(window->DC.CursorPosPrevLine.x + g.Style.ItemSpacing.x, window->DC.CursorPosPrevLine.y);
            if (window->WorkRect.Contains(ImRect(pos, ImVec2(pos.x + size.x, pos.y + size.y))))
            {
                ImGui::SameLine();
            }
        }

        inline void draw_flag(const char* name, e_flags flags)
        {
            ImGuiContext& g = *GImGui;
            ImVec2 size(ImGui::GetFrameHeight() + g.Style.ItemInnerSpacing.x + ImGui::CalcTextSize(name).x, ImGui::GetFrameHeight());
            same_line_or_wrap(size); // FIXME-LAYOUT: To be done automatically once we rework ItemSize/ItemAdd into ItemLayout.

            bool highlight_errors = (flags == e_flags::error && g.DebugLogSkippedErrors > 0);
            if (highlight_errors)
                ImGui::PushStyleColor(ImGuiCol_Text, ImLerp(g.Style.Colors[ImGuiCol_Text], ImVec4(1.0f, 0.0f, 0.0f, 1.0f), 0.30f));
            if (checkbox_flags(name, &m_flags, flags) && g.IO.KeyShift && has_flag(m_flags, flags))
            {
                g.DebugLogAutoDisableFrames = 2;
                //g.DebugLogAutoDisableFlags |= flags;
            }
            if (highlight_errors)
            {
                ImGui::PopStyleColor();
                ImGui::SetItemTooltip("%d past errors skipped.", g.DebugLogSkippedErrors);
            }
            else
            {
                ImGui::SetItemTooltip("Hold SHIFT when clicking to enable for 2 frames only (useful for spammy log entries)");
            }
        }

		inline void draw()
		{
            ImGuiContext& g = *GImGui;
            if ((g.NextWindowData.HasFlags & ImGuiNextWindowDataFlags_HasSize) == 0)
            {
                ImGui::SetNextWindowSize(ImVec2(0.0f, ImGui::GetFontSize() * 12.0f), ImGuiCond_FirstUseEver);
            }
                
            if (!ImGui::Begin("log", &m_is_open))
            {
                ImGui::End();
                return;
            }
            {
                if (m_is_minimal == false)
                {
                    checkbox_flags("All", &m_flags, e_flags::all);
                    draw_flag("errors", e_flags::error);
                    draw_flag("info", e_flags::info);
                    draw_flag("warning", e_flags::warning);

                    if (ImGui::SmallButton("Clear"))
                    {
                        m_textbuffer.clear();
                        m_textindex.clear();
                        g.DebugLogSkippedErrors = 0;
                    }

                    ImGui::SameLine();
                    if (ImGui::SmallButton("Copy"))
                        ImGui::SetClipboardText(m_textbuffer.c_str());
                    ImGui::SameLine();
                }

                ImGui::BeginChild("##log", ImVec2(0.0f, 0.0f), ImGuiChildFlags_Borders, ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_AlwaysHorizontalScrollbar);
                const e_flags backup_log_flags = m_flags;
                ImGuiListClipper clipper;
                clipper.Begin(m_textindex.size());
                while (clipper.Step())
                {
                    for (int line_no = clipper.DisplayStart; line_no < clipper.DisplayEnd; line_no++)
                    {
                        ImGui::DebugTextUnformattedWithLocateItem(m_textindex.get_line_begin(m_textbuffer.c_str(), line_no), m_textindex.get_line_end(m_textbuffer.c_str(), line_no));
                    }
                }
                m_flags = backup_log_flags;
                if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
                {
                    ImGui::SetScrollHereY(1.0f);
                }
                ImGui::EndChild();
            }
            
            ImGui::End();
		}
	};
}

ENABLE_ENUM_BIT_OPERATORS(influx::imgui::logger::e_flags);
#endif