#pragma once

#ifndef _CORE_BRDF_H_
#define _CORE_BRDF_H_

#include "Core/Math/Math.h"
#include "Core/Math/Vector.h"

namespace influx::BRDF
{
	using Colour = math::Vectorf3;

	namespace NormalDistribution
	{
		inline float TrowbridgeReitz(
			const math::Vectorf3& hitNormal,
			const math::Vectorf3& halfVector,
			float roughness_2)
		{
			float result{};

			float roughness_2_2{ powf(roughness_2, 2.f) };

			float c1{ powf(math::Vectorf3::Dot(hitNormal, halfVector), 2) };
			float c2{ roughness_2_2 - 1 };

			result = roughness_2_2 / (float(math::k_PI) * powf(c1 * c2 + 1, 2.f));

			return result;
		}
	}

	namespace Fresnel
	{
		inline Colour Schlick(
			const math::Vectorf3& halfVector,
			const math::Vectorf3& toView,
			const Colour& albedo,
			bool isMetal)
		{
			Colour F0{ albedo };

			if (!isMetal) F0 = Colour{ 0.4f, 0.4f, 0.4f };

			Colour c1{ Colour{ 1.f, 1.f, 1.f } - F0 };
			float c2{ powf(1 - (math::Vectorf3::Dot(halfVector, toView)), 5.f) };

			return F0 + c1 * c2;
		}
	}

	namespace GeometryFunction
	{
		inline float Schlick(
			const math::Vectorf3& hitNormal,
			const math::Vectorf3& toView,
			float k)
		{
			return math::Vectorf3::Dot(hitNormal, toView) / (math::Vectorf3::Dot(hitNormal, toView) * (1 - k) + k);
		}
	}

	struct PhongSettings
	{
		Colour DiffuseColour;

		float Specular;
		float Diffuse;
		float PhongExponent;
	};

	inline Colour Phong(const PhongSettings& settings,
		const math::Vectorf3& fromLight,
		const math::Vectorf3& lightIntensity,
		const math::Vectorf3& toView,
		const math::Vectorf3& hitNormal)
	{
		float normalDotFromLight = math::Vectorf3::Dot(hitNormal, fromLight);

		// normal must be normalized here!
		math::Vectorf3 reflectVector{ -fromLight + 2 * normalDotFromLight * hitNormal };

		Colour diffuse = settings.DiffuseColour * lightIntensity * math::clamp(normalDotFromLight, 0.0f, 1.0f);
		Colour specular = math::Vectorf3::One() * lightIntensity * powf(math::clamp(math::Vectorf3::Dot(reflectVector, toView), 0.0f, 1.0f), settings.PhongExponent);

		return (settings.Diffuse * diffuse) + (settings.Specular * specular);
	}

	struct CookTorranceSettings
	{
		Colour Albedo;
		float Roughness;
		bool IsMetal;
		bool DirectLighting = true;
	};

	inline Colour CookTorrance(const CookTorranceSettings& settings,
		const math::Vectorf3& toView,
		const math::Vectorf3& fromLight,
		const math::Vectorf3& hitNormal)
	{
		math::Vectorf3 halfVector{ toView + fromLight };
		math::Vectorf3::Normalize(halfVector);
		// angle between either & halfvector can't be bigger than 90° (PI / 2)

		float remappedRoughness{};

		if (settings.DirectLighting) remappedRoughness = powf((settings.Roughness + 1), 2) / 8.f;
		else remappedRoughness = powf(settings.Roughness, 2) / 2.f;

		// 1-line alternative for ifstatement
		remappedRoughness = powf(settings.Roughness + (settings.DirectLighting * 1), 2.f) / 2.f + (settings.DirectLighting * 6);

		float normalDistribution{ NormalDistribution::TrowbridgeReitz(hitNormal, halfVector, powf(settings.Roughness, 2.f)) };
		Colour fresnel{ Fresnel::Schlick(halfVector, toView, settings.Albedo, settings.IsMetal) };
		float geometry{ GeometryFunction::Schlick(hitNormal, toView, remappedRoughness) * GeometryFunction::Schlick(hitNormal, fromLight, remappedRoughness) };

		return (fresnel * geometry * normalDistribution) / (4 * (math::Vectorf3::Dot(toView, hitNormal) * (math::Vectorf3::Dot(fromLight, hitNormal))));
	}
}

#endif