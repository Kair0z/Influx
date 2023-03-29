#include "PixelRaytracer.h"

#include "Core/Math/Math.h"
#include "Core/Math/Random.h"
#include "Core/BRDF/BRDF.h"

#include "PixEvents.h"

namespace Influx
{
	PixelRenderer::PixelOutput Influx::PixelRaytracer::RenderPixel(
		const RenderScene& scene, const Math::Vectorf2& uv, const float ar) const
	{
		Math::Ray ray = CreateViewRay(uv, ar);

		float depth = GetRenderSettings().RenderDepthMinMax.y;
		bool anyHit = false;
		HitRecord record{};

		// For each object:
		size_t sphereHitIdx = 0;
		for (size_t i = 0; i < scene.Spheres.size(); ++i)
		{
			const Math::Sphere<float>& sphere = scene.Spheres[i];

			if (TraceSphere(sphere, ray, record, depth))
			{
				anyHit = true;
				sphereHitIdx = i;
			}
		}

		PixelOutput pixelResult{};
		pixelResult.Depth = record.T;
		pixelResult.AnythingRendered = anyHit;

		if (anyHit)
		{
			switch (GetRenderMode())
			{
			default:
			case ERenderMode::Material:
			{
				BRDF::PhongSettings settings{};
				settings.PhongExponent = 100.0f;
				settings.Specular = 0.7f;
				settings.DiffuseColour = { 0.8f, 0.4f, 0.7f };
				settings.Diffuse = 0.6f;

				settings.DiffuseColour *= scene.Randoms[sphereHitIdx];

				auto result = BRDF::Phong(settings, scene.MainLight.Direction, scene.MainLight.Colour, record.ToView, record.Normal);

				pixelResult.RGBA = { result.r, result.g, result.b, 1.0f };
				break;
			}

			case ERenderMode::Normals:
			{
				Math::Vectorf3 colouredNormal = record.Normal;
				colouredNormal += Math::Vectorf3{ 1.0f, 1.0f, 1.0f };
				colouredNormal *= 0.5f;

				pixelResult.RGBA = { colouredNormal.r, colouredNormal.g, colouredNormal.b, 1.0f };
				break;
			}

			case ERenderMode::Depth:
			{
				float colouredDepth = RemapDepth(record.T);
				pixelResult.RGBA = { colouredDepth, colouredDepth, colouredDepth, 1.0f };
				break;
			}

			}
		}

		return pixelResult;
	}

	Math::Ray PixelRaytracer::CreateViewRay(const Math::Vectorf2& uv, const float ar, float sampleRandStrength) const
	{
		Math::Vectorf3 worldOrigin = GetCameraPosition();
		Math::Vectorf3 worldDirection = GetCameraForward();

		Math::Vectorf2 ndc = (2 * uv) - Math::Vectorf2(1.0f, 1.0f); // [uv:0,1] => [ndc:-1,1]

		if (sampleRandStrength > 0.0f)
		{
			// Apply a random pixel-offset
			worldOrigin.x += (ndc.x + (sampleRandStrength * Random::TentRandom<float>() * ar)) * GetCamera().GetFov() * ar;
			worldOrigin.y += (ndc.y + (sampleRandStrength * Random::TentRandom<float>())) * GetCamera().GetFov();
		}
		else
		{
			worldOrigin.x += (ndc.x) * GetCamera().GetFov() * ar;
			worldOrigin.y += (ndc.y) * GetCamera().GetFov();
		}

		float min = GetRenderSettings().RenderDepthMinMax.x;
		float max = GetRenderSettings().RenderDepthMinMax.y;

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
