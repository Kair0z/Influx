#pragma once
#include "PixelRenderer.h"

#include "Core/Geometry/Ray.h"

namespace influx
{
	class PixelRaytracer : public PixelRenderer
	{
	public:
		PixelRaytracer() = default;

		virtual PixelOutput RenderPixel(const RenderScene& scene, const Math::Vectorf2& uv, const float ar) const override;

	private:
		struct HitRecord
		{
			Math::Vectorf3 Normal;
			Math::Vectorf3 WorldPosition;
			Math::Vectorf3 ToView;
			float T;
			float Dot;
		};

		Math::Ray CreateViewRay(const Math::Vectorf2& uv, const float ar, float sampleRandStrength) const;
		Math::Ray CreateViewRay(const Math::Vectorf2& uv, const float ar) const;
		bool TraceSphere(const Math::Sphere<float>& sphere, const Math::Ray& ray, HitRecord& out_hitRecord, float& depthBuffer) const;
	};
}


