#pragma once

#ifndef _CORE_BRDF_H_
#define _CORE_BRDF_H_

#include "Core/Math/Math.h"
#include "Core/Math/Vector.h"

namespace influx::BRDF
{
	using colour = math::vectorf3;

	namespace normal_distribution
	{
		inline float trowbridge_reitz(
			const math::vectorf3& hitNormal,
			const math::vectorf3& halfVector,
			float roughness_2)
		{
			float result{};

			float roughness_2_2{ powf(roughness_2, 2.f) };
			float c1{ powf(math::vectorf3::dot(hitNormal, halfVector), 2) };
			float c2{ roughness_2_2 - 1 };
			result = roughness_2_2 / (float(math::k_PI) * powf(c1 * c2 + 1, 2.f));

			return result;
		}
	}

	namespace fresnel
	{
		inline colour schlick(
			const math::vectorf3& halfVector,
			const math::vectorf3& toView,
			const colour& albedo,
			bool isMetal)
		{
			colour F0{ albedo };

			if (!isMetal) F0 = colour{ 0.4f, 0.4f, 0.4f };

			colour c1{ colour{ 1.f, 1.f, 1.f } - F0 };
			float c2{ powf(1 - (math::vectorf3::dot(halfVector, toView)), 5.f) };

			return F0 + c1 * c2;
		}
	}

	namespace geometry
	{
		inline float Schlick(
			const math::vectorf3& hitNormal,
			const math::vectorf3& toView,
			float k)
		{
			return math::vectorf3::dot(hitNormal, toView) / (math::vectorf3::dot(hitNormal, toView) * (1 - k) + k);
		}
	}

	struct phong_settings final
	{
		colour m_diffuse_colour;

		float m_specular;
		float m_diffuse;
		float m_phong_exponent;
	};

	inline colour phong(const phong_settings& settings,
		const math::vectorf3& fromLight,
		const math::vectorf3& lightIntensity,
		const math::vectorf3& toView,
		const math::vectorf3& hitNormal)
	{
		float normalDotFromLight = math::vectorf3::dot(hitNormal, fromLight);

		// normal must be normalized here!
		math::vectorf3 reflectVector{ -fromLight + 2 * normalDotFromLight * hitNormal };

		colour diffuse = settings.m_diffuse_colour * lightIntensity * math::clamp(normalDotFromLight, 0.0f, 1.0f);
		colour specular = math::vectorf3::one() * lightIntensity * powf(math::clamp(math::vectorf3::dot(reflectVector, toView), 0.0f, 1.0f), settings.m_phong_exponent);

		return (settings.m_diffuse * diffuse) + (settings.m_specular * specular);
	}

	struct cooktorrance_settings
	{
		colour m_albedo;
		float m_roughness;
		bool m_is_metal;
		bool m_direct_lighting = true;
	};

	inline colour CookTorrance(const cooktorrance_settings& settings,
		const math::vectorf3& toView,
		const math::vectorf3& fromLight,
		const math::vectorf3& hitNormal)
	{
		math::vectorf3 halfVector{ toView + fromLight };
		math::vectorf3::normalize(halfVector);
		// angle between either & halfvector can't be bigger than 90° (PI / 2)

		float remappedRoughness{};

		if (settings.m_direct_lighting) remappedRoughness = powf((settings.m_roughness + 1), 2) / 8.f;
		else remappedRoughness = powf(settings.m_roughness, 2) / 2.f;

		// 1-line alternative for ifstatement
		remappedRoughness = powf(settings.m_roughness + (settings.m_direct_lighting * 1), 2.f) / 2.f + (settings.m_direct_lighting * 6);

		float normalDistribution{ normal_distribution::trowbridge_reitz(hitNormal, halfVector, powf(settings.m_roughness, 2.f)) };
		colour fresnel{ fresnel::schlick(halfVector, toView, settings.m_albedo, settings.m_is_metal) };
		float geometry{ geometry::Schlick(hitNormal, toView, remappedRoughness) * geometry::Schlick(hitNormal, fromLight, remappedRoughness) };

		return (fresnel * geometry * normalDistribution) / (4 * (math::vectorf3::dot(toView, hitNormal) 
			* (math::vectorf3::dot(fromLight, hitNormal))));
	}
}

#endif