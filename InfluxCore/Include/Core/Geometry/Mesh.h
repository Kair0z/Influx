#pragma once

#ifndef __CORE_GEOMETRY_MESH_H_
#define __CORE_GEOMETRY_MESH_H_

#include "Core/Geometry/Triangle.h"
#include "Core/Container/Containers.h"

namespace Influx
{
	struct Mesh final
	{
		Vector<Math::Trianglef> Triangles;
	};
}

#endif