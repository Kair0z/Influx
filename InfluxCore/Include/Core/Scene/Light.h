#pragma once

#ifndef __CORE_SCENE_LIGHT_H_
#define __CORE_SCENE_LIGHT_H_

namespace Influx::Scene
{
	enum class ELightType
	{
		Unknown,
		Directional,
		Point,
		Spot,
		Max
	};

	class Light
	{
	public:

	private:

	};

	namespace Internal
	{
		template <ELightType _E>
		class TLight final : public Light
		{

		};
	}

	using DirectionalLight = Internal::TLight<ELightType::Directional>;
	using PointLight = Internal::TLight<ELightType::Point>;
	using SpotLight = Internal::TLight<ELightType::Spot>;
}

#endif