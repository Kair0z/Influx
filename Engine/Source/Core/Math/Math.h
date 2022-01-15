#pragma once

#ifndef _MATH_H_
#define _MATH__H_

#ifdef max
#undef max
#endif

#include <GLM/glm/vec2.hpp>
#include <GLM/glm/vec3.hpp>
#include <GLM/glm/vec4.hpp>
		  
#include <GLM/glm/mat3x3.hpp>
#include <GLM/glm/mat4x4.hpp>

namespace Influx
{
	namespace Math
	{
		template<uint8_t _N>
		using VectorU = glm::vec<_N, unsigned int, glm::defaultp>;

		using Vector2f = glm::vec2;
		using Vector2u = glm::uvec2;

		using Vector3f = glm::vec3;
		using Vector3u = glm::uvec3;

		using Vector4f = glm::vec4;
		using Vector4u = glm::uvec4;

		using Matrix4x4 = glm::mat4x4;
		using Matrix3x3 = glm::mat3x3;

		struct Rectf final
		{
			inline Rectf(float l, float b, float w, float h) : LB{l,b}, WH{w,h}{}

			Vector2f LB{}; // Left-Bottom
			Vector2f WH{}; // Width-Height
		};

		struct Transform3D final
		{
			inline Transform3D(const Vector3f& position, const Vector3f& rotation, const Vector3f& scale) : Position{position}, Rotation{rotation}, Scale{scale}{}

			Vector3f Position;
			Vector3f Rotation;
			Vector3f Scale;
		};

		template <typename T>
		constexpr inline T Lerp(const T& t, const T& min, const T& max)
		{
			return (max - min) * t;
		}

		template <typename T>
		inline T Max(const T& a, const T& b)
		{
			return (a < b) ? b : a;
		}

		inline Vector3f HueToRGB(float hue) {
			float r = abs(hue * 6 - 3) - 1; //red
			float g = 2 - abs(hue * 6 - 2); //green
			float b = 2 - abs(hue * 6 - 4); //blue
			Vector3f rgb = Vector3f(r, g, b); //combine components
			rgb = glm::clamp(rgb, 0.0f, 1.0f); //clamp between 0 and 1
			return rgb;
		}
	}
}

#endif


