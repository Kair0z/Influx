#pragma once

#ifndef _CORE_BRDF_H_
#define _CORE_BRDF_H_

#include "Core/Math/Math.h"
#include "Core/Math/Vector.h"

namespace Influx::BRDF
{
	using Colour = Math::Vectorf3;

	namespace NormalDistribution
	{
		inline float TrowbridgeReitz(
			const Math::Vectorf3& hitNormal,
			const Math::Vectorf3& halfVector,
			float roughness_2)
		{
			float result{};

			float roughness_2_2{ powf(roughness_2, 2.f) };

			float c1{ powf(Math::Vectorf3::Dot(hitNormal, halfVector), 2) };
			float c2{ roughness_2_2 - 1 };

			result = roughness_2_2 / (float(Math::k_PI) * powf(c1 * c2 + 1, 2.f));

			return result;
		}
	}

	namespace Fresnel
	{
		inline Colour Schlick(
			const Math::Vectorf3& halfVector,
			const Math::Vectorf3& toView,
			const Colour& albedo,
			bool isMetal)
		{
			Colour F0{ albedo };

			if (!isMetal) F0 = Colour{ 0.4f, 0.4f, 0.4f };

			Colour c1{ Colour{ 1.f, 1.f, 1.f } - F0 };
			float c2{ powf(1 - (Math::Vectorf3::Dot(halfVector, toView)), 5.f) };

			return F0 + c1 * c2;
		}
	}

	namespace GeometryFunction
	{
		inline float Schlick(
			const Math::Vectorf3& hitNormal,
			const Math::Vectorf3& toView,
			float k)
		{
			return Math::Vectorf3::Dot(hitNormal, toView) / (Math::Vectorf3::Dot(hitNormal, toView) * (1 - k) + k);
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
		const Math::Vectorf3& fromLight,
		const Math::Vectorf3& lightIntensity,
		const Math::Vectorf3& toView,
		const Math::Vectorf3& hitNormal)
	{
		float normalDotFromLight = Math::Vectorf3::Dot(hitNormal, fromLight);

		// normal must be normalized here!
		Math::Vectorf3 reflectVector{ -fromLight + 2 * normalDotFromLight * hitNormal };

		Colour diffuse = settings.DiffuseColour * lightIntensity * Math::Clamp(normalDotFromLight, 0.0f, 1.0f);
		Colour specular = Math::Vectorf3::One() * lightIntensity * powf(Math::Clamp(Math::Vectorf3::Dot(reflectVector, toView), 0.0f, 1.0f), settings.PhongExponent);

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
		const Math::Vectorf3& toView,
		const Math::Vectorf3& fromLight,
		const Math::Vectorf3& hitNormal)
	{
		Math::Vectorf3 halfVector{ toView + fromLight };
		Math::Vectorf3::Normalize(halfVector);
		// angle between either & halfvector can't be bigger than 90° (PI / 2)

		float remappedRoughness{};

		if (settings.DirectLighting) remappedRoughness = powf((settings.Roughness + 1), 2) / 8.f;
		else remappedRoughness = powf(settings.Roughness, 2) / 2.f;

		// 1-line alternative for ifstatement
		remappedRoughness = powf(settings.Roughness + (settings.DirectLighting * 1), 2.f) / 2.f + (settings.DirectLighting * 6);

		float normalDistribution{ NormalDistribution::TrowbridgeReitz(hitNormal, halfVector, powf(settings.Roughness, 2.f)) };
		Colour fresnel{ Fresnel::Schlick(halfVector, toView, settings.Albedo, settings.IsMetal) };
		float geometry{ GeometryFunction::Schlick(hitNormal, toView, remappedRoughness) * GeometryFunction::Schlick(hitNormal, fromLight, remappedRoughness) };

		return (fresnel * geometry * normalDistribution) / (4 * (Math::Vectorf3::Dot(toView, hitNormal) * (Math::Vectorf3::Dot(fromLight, hitNormal))));
	}
}

#endif