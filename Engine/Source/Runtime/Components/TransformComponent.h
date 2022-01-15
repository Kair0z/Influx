#pragma once
#include "Runtime/Components/Component.h"
#include "Core/Math/Math.h"

namespace Influx
{
	using namespace Math;
	class TransformComponent final : public Component
	{
	public:
		inline TransformComponent(const Vector3f& position, const Vector3f& rotation, const Vector3f& scale) 
			: mTransform{position, rotation, scale}{}

	private:
		Math::Transform3D mTransform;
	};
}


