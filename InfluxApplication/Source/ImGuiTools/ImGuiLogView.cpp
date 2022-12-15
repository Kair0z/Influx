#include "ImGuiLogView.h"

#include <vadefs.h>

namespace Influx::Application
{
	void ImGuiLogView::AddLog(const char* fmt, ...)
	{
        int old_size = m_buffer.size();
        va_list args;
        va_start(args, fmt);
        m_buffer.appendfv(fmt, args);
        va_end(args);
        for (int new_size = m_buffer.size(); old_size < new_size; old_size++)
            if (m_buffer[old_size] == '\n')
                m_lineOffsets.push_back(old_size);
        m_scrollToBottom = true;
	}

    void ImGuiLogView::Draw(const char* title, bool* opened)
    {
        ImGui::SetNextWindowSize(ImVec2(500, 400));
        ImGui::Begin(title, opened);
        if (ImGui::Button("Clear")) Clear();
        ImGui::SameLine();
        bool copy = ImGui::Button("Copy");
        ImGui::SameLine();
        m_filter.Draw("Filter", -100.0f);
        ImGui::Separator();
        ImGui::BeginChild("scrolling");
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 1));
        if (copy)
        {
            ImGui::LogToClipboard();
        }

        if (m_filter.IsActive())
        {
            const char* buf_begin = m_buffer.begin();
            const char* line = buf_begin;
            for (int line_no = 0; line != NULL; line_no++)
            {
                const char* line_end = (line_no < m_lineOffsets.Size) ? buf_begin + m_lineOffsets[line_no] : NULL;
                if (m_filter.PassFilter(line, line_end))
                {
                    ImGui::TextUnformatted(line, line_end);
                }
                    
                line = line_end && line_end[1] ? line_end + 1 : NULL;
            }
        }
        else
        {
            ImGui::TextUnformatted(m_buffer.begin());
        }

        if (m_scrollToBottom)
        {
            ImGui::SetScrollHereY(1.0f);
        }
            
        m_scrollToBottom = false;
        ImGui::PopStyleVar();
        ImGui::EndChild();
        ImGui::End();
    }

    void ImGuiLogView::Clear()
    {
        m_buffer.clear();
        m_lineOffsets.clear();
    }

}
