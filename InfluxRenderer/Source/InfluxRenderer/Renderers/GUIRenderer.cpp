#include "renderer_pch.h"
#include "InfluxRenderer/Renderers/GUIRenderer.h"

#include "ImGuiTools/ImGuiWidgets.h"

#include "InfluxGraphics/RHI.h"

#include "Core/Platform/WindowsPlatform.h"

// Imgui Win32
#include "ImGui/imgui_impl_win32.h"
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


	// IRenderer Interface:
	void GUIRenderer::OnPostInitializeAPI(const Graphics::EGraphicsAPI api, Graphics::RHIDevice* device)
	{
		using namespace Graphics;

		// ImGui_ImplDX12_Init
		{
			using namespace Graphics;
			
			ImGuiIO& io = ImGui::GetIO();

			// Setup backend capabilities flags:
			// io.BackendRendererUserData = (void*)bd;
			io.BackendRendererName = "imgui_impl_dx12";
			io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;  // We can honor the ImDrawCmd::VtxOffset field, allowing for large meshes.

			// Create fonts texture:
			{
				unsigned char* pixels;
				int width, height;
				io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

				Graphics::RHITextureDesc texDesc{};
				texDesc.Dimensions.x = static_cast<uint32>(width);
				texDesc.Dimensions.y = static_cast<uint32>(height);
				texDesc.Format = Graphics::ERHIFormat::RGBA_8_Unorm;
				texDesc.NumMips = 1u;

				mp_fontTexture = device->CreateTexture(Graphics::ERHIResourceState::CopyDest, texDesc);
			}
			
			// Setup vertex & index buffers:
			{

			}
		}
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

		// ImGui_ImplDX12_RenderDrawData
		{
			if (mp_vertexBufferResource == nullptr)
			{
				
			}
			if (mp_indexBufferResource == nullptr)
			{

			}
		}
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
		ImGui_ImplWin32_Shutdown();
	}

	bool GUIRenderer::IsAttachedToWin32Backend()
	{
		ImGui::CreateContext();
		return ImGui::GetIO().BackendPlatformUserData != nullptr;
	}
}