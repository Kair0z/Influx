
#define PLATFORM_WINDOWS 1
#include "Core/Platform/WindowsPlatform.h"
#include "InfluxRenderer/Renderer.h"

#include "InfluxGraphics/RHISwapchain.h"
#include "InfluxGraphics/RHICommandList.h"

using namespace Influx;

class SceneRenderer final : public Renderer::IRenderer
{
	/* Initializing RHI Resources */
	virtual void Initialize(const RHIDevicePtr device) override
	{

	}

	/* Submitting work onto a passed RHICommandList */
	virtual void OnRender(RHICommandListPtr commandList) const override
	{

	}

	/* Cleaning up RHI Resources */
	virtual void Cleanup(const RHIDevicePtr device) override
	{

	}
};

int main()
{
	Platform::WindowSettings windowSettings{};
	windowSettings.Width = 640u;
	windowSettings.Heigth = 480u;
	windowSettings.Name = "Flux Renderer";
	Platform::WindowHandle wndHandle = Platform::CreateWindow(windowSettings, true);

	const bool vsync = true;

	{
		Renderer::RootRenderer rootRenderer{};
		rootRenderer.SetGraphicsAPI(Graphics::EGraphicsAPI::D3D12);
		rootRenderer.AttachToWindow(wndHandle);

		while (Platform::PollWindowEvents(wndHandle))
		{
			rootRenderer.Render([](Graphics::RHICommandList* cmdList)
			{
				// use cmdList...
			});

			rootRenderer.Present(vsync);
		}
	}
}