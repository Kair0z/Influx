#pragma once

#include "Runtime/Rendering/Renderer.h"
#include "D3D12API.h"
#include "D3D12Conversion.h"

namespace Influx::Editor
{
	class EditorRenderer : public RenderInterface
	{
	public:
		virtual ~EditorRenderer() = default;
	};

	class D3D12EditorRenderer final : public EditorRenderer
	{
	public:
		virtual void InitializeRHI(const Graphics::GraphicsAPI* gfxApi) override final;
		virtual void OnRender(Graphics::RHICommandList* cmdList, Graphics::RHITexture* gameRenderTexture) override final;

	private:
		Graphics::RHIDescriptorHeap* FontDescriptorHeap;
	};
}


