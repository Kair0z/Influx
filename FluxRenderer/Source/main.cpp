
#define PLATFORM_WINDOWS 1
#include "Core/Platform/WindowsPlatform.h"

#include "InfluxRenderer/Renderer.h"

#include "InfluxGraphics/RHISwapchain.h"
#include "InfluxGraphics/RHICommandList.h"

#include "Renderer/GUIRenderer.h"

using namespace Influx;

int main()
{
	Platform::WindowSettings windowSettings{};
	windowSettings.Width = 640u;
	windowSettings.Heigth = 480u;
	windowSettings.Name = "Flux Renderer";
	Platform::WindowHandle wndHandle = Platform::CreateWindow(windowSettings, true);

	const bool vsync = true;

	{
		GUI::GUIRenderer guiRenderer{};

		Renderer::RootRenderer rootRenderer{};
		rootRenderer.SetGraphicsAPI(Graphics::EGraphicsAPI::D3D12);
		rootRenderer.AttachToWindow(wndHandle);

		// Add GUI renderer
		rootRenderer.GetChildRendererList().push_back(&guiRenderer);

		while (Platform::PollWindowEvents(wndHandle))
		{
			rootRenderer.Render();
			rootRenderer.Present(vsync);
		}
	}
}