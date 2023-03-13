#pragma once

#include "InfluxRenderer/IRenderer.h"

#include "Core/Scene/Scene.h"

namespace Influx::Renderer
{
	class SceneRenderer final : public Renderer::IRenderer
	{
	public:
		struct CameraData final
		{
			CameraData() = default;
			CameraData(const Math::Vectorf3& position, const Math::Vectorf3& forward, float fov)
				: Position{ position }, Forward{ forward }, Fov{ fov } {};

			float Fov;
			Math::Vectorf3 Position;
			Math::Vectorf3 Forward;
		};
		void SetCamera(const CameraData& cameraData);

		struct LightData final
		{
			LightData() = default;
			LightData(const Math::Vectorf3& position, const Math::Vectorf3& forward, const Math::Vectorf3& colour, float intensity)
				: Position{ position }, Forward{ forward }, Colour{ colour }, Intensity{ intensity } {};

			Math::Vectorf3 Position;
			Math::Vectorf3 Forward;
			
			Math::Vectorf3 Colour;
			float Intensity;
		};
		void AddLight(const LightData& lightData);

		struct MeshData final
		{
			MeshData() = default;
			MeshData(Scene::Mesh data)
				: m_meshData{ data } {}

			Scene::Mesh m_meshData;
		};
		void AddMesh(const MeshData& meshData);

		struct MaterialData final
		{

		};
		void AddMaterial(const MaterialData& material);

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

		CameraData m_cameraData;
		Vector<LightData> m_lights;
		Vector<MeshData> m_meshes;
		Vector<MaterialData> m_materials;
	};
}


