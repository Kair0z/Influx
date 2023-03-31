
#include "InfluxAssets/InfluxAssets.h"
#include "RHI/RHIRenderer.h"
#include "Core/Platform/WindowsPlatform.h"

int main()
{
	using namespace Influx;

	// [SETTINGS]
	constexpr bool Vsync = true;
	const Influx::Math::Vectoru2 WindowDimensions{ 640u, 480u };
	const float AspectRatio = (float)WindowDimensions.x / (float)WindowDimensions.y;
	uint64 NumFrames = 4096 * 4096;

	IFluxRenderer* renderer = new RHIRenderer();

	// [Compile Shaders]
	if (Assets::ShaderData shaderData{}; Assets::LoadShaderFile("E:/Git/Influx/FluxRenderer/Shaders/shaders.hlsl", shaderData))
	{
		renderer->SetMaterial({ shaderData.VertexShader, shaderData.PixelShader });
	}

	// [Get Scene Data]
	if (Assets::Scene leblancScene{}; Assets::LoadSceneFile("E:/Git/Influx/Resources/Meshes/box.fbx", leblancScene))
	{
		for (uint64 s = 0; s < leblancScene.Meshes.size(); ++s)
		{
			renderer->AddMesh(leblancScene.Meshes[s]);
		}
	}

	// [Setup hardcoded Camera]
	renderer->SetCameraData(Scene::Camera{90.0f, 0.001f, 1000.0f});
	renderer->SetCameraTransform(Math::Vectorf3{} , Math::Vectorf3{});

	// [Create Window]
	Platform::WindowSettings	windowSettings(WindowDimensions, "Flux Renderer");
	Platform::WindowHandle		wndHandle = Platform::CreateWindow(windowSettings, true);

	while ((NumFrames--) != 0u)
	{
		/* We should be able to build render-work once... */
		renderer->BuildRenderWork(wndHandle);

		renderer->PresentToWindow(wndHandle);
	}

	delete renderer;
}