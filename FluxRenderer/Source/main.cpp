
#include "InfluxAssets/InfluxAssets.h"
#include "Core/Platform/WindowsPlatform.h"
#include "Core/Geometry/Vertex.h"

using namespace Influx;

namespace Settings
{
	bool Vsync = true;

	const Influx::Math::Vectoru2 WindowDimensions{ 640u, 480u };
	const float AspectRatio = (float)WindowDimensions.x / (float)WindowDimensions.y;
}

struct Shaders final
{
	using ByteCode = Vector<byte>;

	ByteCode VertexShader;
	ByteCode PixelShader;
};

struct SceneData final
{
	using Index = uint64;

	Vector<Math::Vertex> VertexBuffer;
	Vector<Index> IndexBuffer;
};

#include "RHI/RHIRenderer.h"

#include <d3dcompiler.h>

int main()
{
	IFluxRenderer* renderer = new RHIRenderer();

	// [Compile Shaders]
	Shaders shaders{};
	{
#if defined(_DEBUG)
		// Enable better shader debugging with the graphics debugging tools.
		UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
		UINT compileFlags = 0;
#endif

		ID3DBlob* vertexShader;
		ID3DBlob* pixelShader;

		HRESULT result{};
		result = D3DCompileFromFile(L"E:/Git/Influx/FluxRenderer/Shaders/shaders.hlsl", nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &vertexShader, nullptr);
		result = D3DCompileFromFile(L"E:/Git/Influx/FluxRenderer/Shaders/shaders.hlsl", nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pixelShader, nullptr);

		for (uint32 i = 0; i < vertexShader->GetBufferSize(); ++i)
		{
			shaders.VertexShader.push_back(reinterpret_cast<uint8*>(vertexShader->GetBufferPointer())[i]);
		}

		for (uint32 i = 0; i < pixelShader->GetBufferSize(); ++i)
		{
			shaders.PixelShader.push_back(reinterpret_cast<uint8*>(pixelShader->GetBufferPointer())[i]);
		}
	}
	renderer->SetMaterial({ shaders.VertexShader, shaders.PixelShader });

	// [Get Scene Data]
	SceneData sceneData{};
	if (Assets::Scene leblancScene{}; Assets::LoadScene("E:/Git/Influx/Resources/Meshes/box.fbx", leblancScene))
	{
		for (uint64 s = 0; s < leblancScene.Meshes.size(); ++s)
		{
			renderer->AddMesh(leblancScene.Meshes[s]);
		}
	}
	renderer->SetCameraData(Scene::Camera{90.0f, 0.001f, 1000.0f});
	renderer->SetCameraTransform(Math::Vectorf3{} , Math::Vectorf3{});

	// [Create Window]
	Platform::WindowSettings	windowSettings(Settings::WindowDimensions, "Flux Renderer");
	Platform::WindowHandle		wndHandle = Platform::CreateWindow(windowSettings, true);

	while (true)
	{
		/* We should be able to build render-work once... */
		renderer->BuildRenderWork(wndHandle);

		renderer->PresentToWindow(wndHandle);
	}

	delete renderer;
}