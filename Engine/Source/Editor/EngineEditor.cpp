#include "pch.h"

#if WITH_EDITOR
#include "Runtime/Engine/Engine.h"
#include "Editor/Editor.h"

#include "Runtime/Engine/GameThread.h"
#include "Runtime/Rendering/RenderThread.h"

namespace Influx
{
	void Engine::OnEditorRender() const
	{
		constexpr static float msTarget = 16.6666f;

		bool isOpen = true;
		ImGui::Begin("Engine Info: ", &isOpen, ImGuiWindowFlags_AlwaysAutoResize);

		if (ImGui::CollapsingHeader("Threads: "))
		{
			ImVec4 red = ImVec4{ 1.0f, 0.0f, 0.0f, 1.0f };
			ImVec4 green = ImVec4{ 0.0f, 1.0f, 0.0f, 1.0f };

			// Gamethread:
			{
				float gtMs = mpGameThread->GetMs();
				ImGui::PushStyleColor(ImGuiCol_Text, (gtMs > msTarget) ? red : green);
				ImGui::Text("Game Thread: %f ms", gtMs);
				//ImGui::Text("Game Thread: %f [stalled]", mpGameThread->GetStallMs());
				ImGui::PopStyleColor();
			}

			// Renderthread:
			{
				float rtMs = mpRenderThread->GetMs();
				ImGui::PushStyleColor(ImGuiCol_Text, (rtMs > msTarget) ? red : green);
				ImGui::Text("Render Thread: %f ms", rtMs);
				//ImGui::Text("Game Thread: %f [stalled]", mpGameThread->GetStallMs());
				ImGui::PopStyleColor();
			}
		}

		if (ImGui::CollapsingHeader("Game Info: "))
		{
			ImGui::Text("DeltaSeconds: %f", mpGameThread->GetDeltaTime());
		}

		if (ImGui::CollapsingHeader("[TODO] System Info: "))
		{
			// [TODO]
		}

		if (ImGui::CollapsingHeader("[TODO] Memory: "))
		{
			// [TODO]
		}

		ImGui::End();
	}
}
#endif