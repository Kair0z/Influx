#include "renderer_pch.h"
#include "InfluxRenderer/Renderers/GUIRenderer.h"
#include "InfluxRenderer/RootRenderer.h"

#include "ImGuiTools/ImGuiWidgets.h"

#include "InfluxGraphics/RHI.h"
#include "InfluxGraphics/D3D12/D3D12CommandList.h"

#include "Core/Platform/WindowsPlatform.h"

// Imgui Win32
#include "ImGui/imgui_impl_win32.h"
#include "ImGui/imgui_impl_dx12.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Influx::GUI
{
	void GUIRenderer::AddWidget(WidgetPtr widget, bool allowDuplicate)
	{
		if (HasWidget(widget) && !allowDuplicate)
		{
			return;
		}

		m_widgetList.push_back(widget);
	}

	void GUIRenderer::RemoveWidget(WidgetPtr widget)
	{
		m_widgetList.remove(widget);
	}

	bool GUIRenderer::HasWidget(WidgetPtr widget) const
	{
		return std::find(m_widgetList.cbegin(), m_widgetList.cend(), widget) != m_widgetList.cend();
	}


	void GUIRenderer::OnPostInitializeAPI(const Graphics::EGraphicsAPI api, Graphics::RHIDevice* device)
	{
		// ...
	}

	void GUIRenderer::OnBuildRenderCommandList(const Renderer::RenderContext& context, Graphics::RHICommandList* cmdList)
	{
		using namespace Graphics;

		ImGui::NewFrame();
		{
			ImGui::ShowDemoWindow();

			for (const WidgetPtr widget : m_widgetList)
			{
				widget->Render();
			}
		}
		ImGui::Render();

		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), ((D3D12CommandList*)cmdList)->GetDxCommandList());
	}

	void GUIRenderer::OnAttachToWindow(const Renderer::RenderContext& context)
	{
		AttachToWin32Backend(context.GetSwapchain()->GetWindowHandle());
	}

	void GUIRenderer::OnDetachFromWindow(const Renderer::RenderContext& context)
	{
		DetachFromWin32Backend();
	}

	void GUIRenderer::OnWindowResize(const Renderer::RenderContext& context, const Math::Vectoru2& oldSize, const Math::Vectoru2& newSize)
	{
		// ...
	}

	void GUIRenderer::OnPreCleanupAPI(const Graphics::EGraphicsAPI api, Graphics::RHIDevice* device)
	{
		// ImGui_ImplDX12_Shutdown
		{
			// Release our vertex & indexbuffers...
		}

		ImGui::DestroyContext();
	}


	void GUIRenderer::AttachToWin32Backend(Platform::WindowHandle windowHandle)
	{
		if (IsAttachedToWin32Backend())
		{
			return;
		}

		ImGui_ImplWin32_Init((void*)windowHandle);
	}

	void GUIRenderer::DetachFromWin32Backend()
	{
		if (IsAttachedToWin32Backend())
		{
			ImGui_ImplWin32_Shutdown();
		}
	}

	bool GUIRenderer::IsAttachedToWin32Backend()
	{
		ImGui::CreateContext();
		return ImGui::GetIO().BackendPlatformUserData != nullptr;
	}
}