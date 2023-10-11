#pragma once

#include "Core/Math/Math.h"
#include "Core/Scene/Camera.h"
#include "Core/Geometry/Sphere.h"
#include "Core/Geometry/Plane.h"

#include <vector>

namespace influx
{
	struct RenderScene
	{
		using Spheref = influx::Math::Sphere<float>;
		std::vector<float> Randoms;
		std::vector<Spheref> Spheres;

		struct DirectionLight
		{
			Math::Vectorf3 Direction;
			Math::Vectorf3 Colour;
		};

		DirectionLight MainLight;
	};

	class PixelRenderer
	{
	public:
		struct PixelOutput
		{
			influx::Math::vector<float, 4u> RGBA;
			float Depth;
			bool AnythingRendered;
		};

		enum class ERenderMode
		{
			Material,
			Depth,
			Normals
		};

		struct RenderSettings
		{
			influx::Math::vector<float, 2u> RenderDepthMinMax = {0.0f, FLT_MAX};
		};

	public:
		virtual PixelOutput RenderPixel(const RenderScene& scene, const Math::Vectorf2& uv, const float ar) const = 0;

		void SetCameraFieldOfView(const float newFov) { m_camera.SetFov(newFov); }
		void SetCameraPosition(const Math::Vectorf3& newPosition) { m_cameraPosition = newPosition; }
		void SetCameraForward(const Math::Vectorf3& newForward) { m_cameraForward = newForward; }
		void SetRenderMode(const ERenderMode renderMode) { m_renderMode = renderMode; }

		const Math::Vectorf3& GetCameraPosition() const { return m_cameraPosition; }
		const Math::Vectorf3& GetCameraForward() const { return m_cameraForward; }
		
		const Scene::Camera& GetCamera() const { return m_camera; }
		const ERenderMode GetRenderMode() const { return m_renderMode; }

		RenderSettings& GetRenderSettings() { return m_renderSettings; }
		const RenderSettings& GetRenderSettings() const { return m_renderSettings; }

	protected:
		inline float RemapDepth(float depthValue) const
		{
			return influx::Math::Remap(depthValue,
				m_renderSettings.RenderDepthMinMax.x, m_renderSettings.RenderDepthMinMax.y,
				0.0f, 1.0f);
		}

	private:
		Scene::Camera m_camera;
		Math::Vectorf3 m_cameraPosition;
		Math::Vectorf3 m_cameraForward;

		ERenderMode m_renderMode;
		RenderSettings m_renderSettings;

	public:
		PixelRenderer() = default;
		PixelRenderer(const PixelRenderer&) = delete;
		PixelRenderer(PixelRenderer&&) = delete;
		PixelRenderer& operator=(const PixelRenderer&) = delete;
		PixelRenderer& operator=(PixelRenderer&&) = delete;
		virtual ~PixelRenderer() = default;
	};
}


