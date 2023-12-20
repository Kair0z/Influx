
// influx assets
#include "InfluxAssets/InfluxAssets.h"

// core
#include "core/platform/windows_platform.h"
#include "core/time.h"
#include "core/threading/ThreadPool.h"

// stl
#include <iostream>

#define _SHADERS_RESOURCE_PATH	"E:/Git/influx/Resources/Shaders/shaders.hlsl"
#define _GEOMETRY_RESOURCE_PATH "E:/Git/influx/Resources/Meshes/box.fbx"

#include "RHI/RHIRenderer.h"
#include "Dx12/Dx12Renderer.h"
#include "Vulkan/VulkanRenderer.h"

int main()
{
	using namespace influx;

	// [SETTINGS]
	constexpr bool Vsync							= true;
	const influx::math::vectoru2 WindowDimensions	= { 640u, 480u };
	const float AspectRatio							= (float)WindowDimensions.x / (float)WindowDimensions.y;
	constexpr uint64 NumFrames						= 6000u;

	Assets::ShaderData shaderData{};
	Assets::Scene sceneData{};

	influx::RunAsync<2u>
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

	for (uint64 s = 0; s < sceneData.Meshes.dimension(); ++s)
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
	platform::create_window_args	windowSettings(WindowDimensions, "Flux Renderer");
	platform::window_handle		wndHandle = platform::CreateWindow(windowSettings, true);

	/* We should be able to build render-work once... */
	renderer->RecordRenderCommands(wndHandle);

	auto before = time::get_now();
	uint64 frameIndex = NumFrames;
	while ((frameIndex--) != 0u)
	{
		renderer->PresentToWindow(wndHandle);
	}

	const float totalMs = time::get_ms_between<float>(time::get_now(), before);
	const float fps = (NumFrames) / (totalMs * 0.001f);

	std::cout << "Num Ms to render " << NumFrames << " frames: " << totalMs << "\n";
	std::cout << "FPS: " << fps << "\n";

	std::cin.get();
	
	delete renderer;
}