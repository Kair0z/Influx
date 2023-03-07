#pragma once

#include "InfluxRenderer/IRenderer.h"

#include "Core/Scene/Scene.h"

namespace Influx::Renderer
{
	class SceneRenderer final : public Renderer::IRenderer
	{
	public:
		void SetSceneToRender(const Influx::Scene::Scene& scene);
		const Influx::Scene::Scene& GetSceneToRender() const;

	private:
		/* After initializing the RHI API Device */
		/* Here you initialize / create your RHI objects */
		virtual void OnPostInitializeAPI(const Graphics::EGraphicsAPI api, Graphics::RHIDevice* device) override;

		/* */
		virtual void OnBuildRenderCommandList(
			const Renderer::RenderContext& context,
			Graphics::RHICommandList* cmdList) override;

		/* */
		virtual void OnWindowResize(
			const Renderer::RenderContext& context,
			const Math::Vectoru2& oldSize, const Math::Vectoru2& newSize) override;

		/* Before cleaning up the RHI API Device */
		/* Here you Release your RHI objects */
		virtual void OnPreCleanupAPI(const Graphics::EGraphicsAPI api, Graphics::RHIDevice* device) override;

	private:
		Graphics::RHIGraphicsPipelineLayout* mp_pipelineLayout;
		Graphics::RHIGraphicsPipeline* mp_pipeline;
		Graphics::RHITexture* mp_sceneColourTexture;

		Graphics::RHIResource* mp_vertexBufferResource;
		Graphics::RHIResource* mp_indexBufferResource;

		Vector<byte> m_compiledVertexShader{};
		Vector<byte> m_compiledPixelShader{};

		Influx::Scene::Scene& m_scene;
	};
}


