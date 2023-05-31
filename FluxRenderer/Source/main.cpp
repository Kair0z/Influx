

#include "RHI/RHIRenderer.h"
#include "Dx12/Dx12Renderer.h"
#include "Vulkan/VulkanRenderer.h"

#include "Core/Platform/WindowsPlatform.h"
#include "Core/Time.h"
#include "InfluxAssets/InfluxAssets.h"
#include "Core/Threading/ThreadPool.h"
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

	Assets::ShaderData shaderData{};
	Assets::Scene sceneData{};

	Influx::RunAsync<2u>
	({
		// [Compile Shaders]
		[&shaderData]()
		{ 
			Assets::LoadShaderFile(_SHADERS_RESOURCE_PATH, shaderData); 
		},

		// [Get Scene Data]
		[&sceneData]()
		{ 
			Assets::LoadSceneFile(_GEOMETRY_RESOURCE_PATH, sceneData); 
		}
	});

	IFluxRenderer* renderer = new Dx12Renderer();

	for (uint64 s = 0; s < sceneData.Meshes.size(); ++s)
	{
		renderer->AddMesh(sceneData.Meshes[s]);
	}

	renderer->SetMaterial(IFluxRenderer::MaterialData{ shaderData.VertexShader, shaderData.PixelShader });

	// [Setup hardcoded Camera]
	const Math::Vectorf3 cameraPosition{ 0.0f, 0.0f, 0.0f };
	const Math::Vectorf3 cameraForward{ 0.0f, 0.0f, -1.0f };
	renderer->SetCameraData(Scene::Camera{90.0f, 0.001f, 1000.0f});
	renderer->SetCameraTransform(cameraPosition, cameraForward);

	// [Create Window]
	Platform::WindowSettings	windowSettings(WindowDimensions, "Flux Renderer");
	Platform::WindowHandle		wndHandle = Platform::CreateWindow(windowSettings, true);

	/* We should be able to build render-work once... */
	renderer->RecordRenderCommands(wndHandle);

	auto before = Time::Now();
	uint64 frameIndex = NumFrames;
	while ((frameIndex--) != 0u)
	{
		renderer->PresentToWindow(wndHandle);
	}

	const float totalMs = Time::MsBetween<float>(Time::Now(), before);
	const float fps = (NumFrames) / (totalMs * 0.001f);

	std::cout << "Num Ms to render " << NumFrames << " frames: " << totalMs << "\n";
	std::cout << "FPS: " << fps << "\n";

	std::cin.get();
	
	delete renderer;
}