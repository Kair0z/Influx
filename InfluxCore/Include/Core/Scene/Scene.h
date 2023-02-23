#pragma once

#ifndef __CORE_SCENE_H_
#define __CORE_SCENE_H_

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
}

#endif