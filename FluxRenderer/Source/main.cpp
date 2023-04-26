
#include "InfluxAssets/InfluxAssets.h"
#include "Dx12/Dx12Renderer.h"

#include "Core/Platform/WindowsPlatform.h"
#include "Core/Time.h"

#include <iostream>

#define _SHADERS_RESOURCE_PATH	"E:/Git/Influx/Resources/Shaders/shaders.hlsl"
#define _GEOMETRY_RESOURCE_PATH "E:/Git/Influx/Resources/Meshes/box.fbx"

int main()
{
	using namespace Influx;

	// [SETTINGS]
	constexpr bool Vsync							= true;
	const Influx::Math::Vectoru2 WindowDimensions	= { 640u, 480u };
	const float AspectRatio							= (float)WindowDimensions.x / (float)WindowDimensions.y;
	constexpr uint64 NumFrames						= 6000u;
	
	IFluxRenderer* renderer = new Dx12Renderer();

	// [Compile Shaders]
	if (Assets::ShaderData shaderData{}; Assets::LoadShaderFile(_SHADERS_RESOURCE_PATH, shaderData))
	{
		renderer->SetMaterial({ shaderData.VertexShader, shaderData.PixelShader });
	}

	// [Get Scene Data]
	if (Assets::Scene leblancScene{}; Assets::LoadSceneFile(_GEOMETRY_RESOURCE_PATH, leblancScene))
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

	/* We should be able to build render-work once... */
	renderer->BuildRenderWork(wndHandle);

	auto before = Time::Now();

	uint64 frameIndex = NumFrames;
	while ((frameIndex--) != 0u)
	{
		renderer->PresentToWindow(wndHandle);
	}

	float totalMs = Time::MsBetween<float>(Time::Now(), before);
	float fps = (NumFrames) / (totalMs * 0.001f);

	std::cout << "Num Ms to render " << NumFrames << " frames: " << totalMs << "\n";
	std::cout << "FPS: " << fps << "\n";

	std::cin.get();
	
	delete renderer;
}