#pragma once

#include "Core/Geometry/Camera.h"
#include "Core/Geometry/Sphere.h"
#include "Core/Geometry/Plane.h"

#include <vector>

namespace Influx
{
	class PixelRenderer
	{
	public:
		using PixelColour = Influx::Math::Vector<uint8_t, 4u>;

	public:
		virtual PixelColour RenderPixel(const std::vector<Math::Sphere<float>>& spheres, const Math::Vectorf2& uv, const float ar) const = 0;

		inline void SetCameraFieldOfView(const float newFov) { m_camera.SetFieldOfView(newFov); }
		inline void SetCameraPosition(const Math::Vectorf3& newPosition) { m_camera.SetPosition(newPosition); }
		inline void SetCameraForward(const Math::Vectorf3& newForward) { m_camera.SetForward(newForward); }

		const Math::Camera& GetCamera() const { return m_camera; }

	private:
		Math::Camera m_camera;

	public:
		PixelRenderer() = default;
		PixelRenderer(const PixelRenderer&) = delete;
		PixelRenderer(PixelRenderer&&) = delete;
		PixelRenderer& operator=(const PixelRenderer&) = delete;
		PixelRenderer& operator=(PixelRenderer&&) = delete;
		virtual ~PixelRenderer() = default;
	};
}


