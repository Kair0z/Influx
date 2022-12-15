#pragma once

#include "../../ImGui/imgui.h"
#include <vadefs.h>

namespace Influx::Application
{
	class ImGuiLogView final
	{
		ImGuiTextBuffer m_buffer;
		ImGuiTextFilter m_filter;
		ImVector<int> m_lineOffsets;
		bool m_scrollToBottom;

	public:
		void AddLog(const char* fmt, ...);
		void Draw(const char* title, bool* opened = nullptr);
		void Clear();

		ImGuiLogView() = default;
		ImGuiLogView(const ImGuiLogView&) = default;
		ImGuiLogView(ImGuiLogView&&) = default;
		ImGuiLogView& operator=(const ImGuiLogView&) = default;
		ImGuiLogView& operator=(ImGuiLogView&&) = default;
		virtual ~ImGuiLogView() = default;
	};
}