#include "PixelRaytracer.h"

#include "Core/Math/Math.h"
#include "Core/Math/Random.h"
#include "Core/BRDF/BRDF.h"

#include "PixEvents.h"

namespace Influx
{
	PixelRenderer::PixelColour Influx::PixelRaytracer::RenderPixel(
		const std::vector<Math::Sphere<float>>& spheres, const Math::Vectorf2& uv, const float ar) const
	{
#ifdef PROFILE
		PIXScopedEvent(0, "PixelRaytracer::RenderPixel");
#endif
		Math::Ray ray = CreateViewRay(uv, ar);

		float depth = FLT_MAX;
		bool anyHit = false;
		HitRecord record{};

		// For each object:
		for (size_t i = 0; i < spheres.size(); ++i)
		{
			const Math::Sphere<float>& sphere = spheres[i];

			anyHit = anyHit || TraceSphere(sphere, ray, record, depth);
		}

		if (anyHit)
		{
			// Material:
			BRDF::PhongSettings settings{};
			settings.PhongExponent = 1.0f;
			settings.SpecularReflectance = 0.5f;

			auto result = BRDF::Phong(settings, Math::Vectorf3{ -0.3, -0.3, 0.3f }, record.ToView, record.Normal);
			return PixelColour{result.r * 255.0f, result.g * 255.0f, result.b * 255.0f, 255.0f };
		}

		return PixelColour{};
	}

	Math::Ray PixelRaytracer::CreateViewRay(const Math::Vectorf2& uv, const float ar, float sampleRandStrength) const
	{
		Math::Vectorf3 worldOrigin = GetCamera().GetPosition();
		Math::Vectorf3 worldDirection = GetCamera().GetForward();

		Math::Vectorf2 ndc = (2 * uv) - Math::Vectorf2(1.0f, 1.0f); // [uv:0,1] => [ndc:-1,1]

		if (sampleRandStrength > 0.0f)
		{
			// Apply a random pixel-offset
			worldOrigin.x += (ndc.x + (sampleRandStrength * Random::TentRandom<float>() * ar)) * GetCamera().GetFieldOfView() * ar;
			worldOrigin.y += (ndc.y + (sampleRandStrength * Random::TentRandom<float>())) * GetCamera().GetFieldOfView();
		}
		else
		{
			worldOrigin.x += (ndc.x) * GetCamera().GetFieldOfView() * ar;
			worldOrigin.y += (ndc.y) * GetCamera().GetFieldOfView();
		}

		float min = 0.0f;
		float max = FLT_MAX;

		return Math::Ray(worldOrigin, worldDirection, min, max);
	}

	Math::Ray PixelRaytracer::CreateViewRay(const Math::Vectorf2& uv, const float ar) const
	{
		return CreateViewRay(uv, ar, 0.0f);
	}

	bool PixelRaytracer::TraceSphere(const Math::Sphere<float>& sphere, const Math::Ray& ray, PixelRaytracer::HitRecord& out_hitRecord, float& depthBuffer) const
	{
		float a = Math::Vectorf3::Dot(ray.GetDirection(), ray.GetDirection());
		float b = Math::Vectorf3::Dot(2 * ray.GetDirection(), ray.GetOrigin() - sphere.m_position);
		float c = Math::Vectorf3::Dot(ray.GetOrigin() - sphere.m_position, ray.GetOrigin() - sphere.m_position) - (sphere.m_radius * sphere.m_radius);

		float d{ (b * b) - 4 * a * c };

		if (d <= 0.0f) return false;

		float dSqrt{ sqrtf(d) };

		float t1{ (-b - dSqrt) / 2.f * a };
		float t2;

		if (t1 < ray.GetMin())
		{
			t2 = (-b + dSqrt) / 2.f * a;
		}

		if (t1 > ray.GetMax() || t1 < ray.GetMin()) return false;

		// Depth
		if (t1 > depthBuffer) return false;

		// Fill out hitrecord
		out_hitRecord.WorldPosition = ray.GetOrigin() + ray.GetDirection() * t1;
		out_hitRecord.Normal = (out_hitRecord.WorldPosition - sphere.m_position).Normalized();
		out_hitRecord.Dot = Math::Vectorf3::Dot(ray.GetDirection(), out_hitRecord.Normal);
		out_hitRecord.T = t1;
		out_hitRecord.ToView = (ray.GetOrigin() - out_hitRecord.WorldPosition).Normalized();

		depthBuffer = t1;
		return true;
	}
}

