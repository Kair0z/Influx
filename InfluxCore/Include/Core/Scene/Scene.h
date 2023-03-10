#pragma once

#ifndef __CORE_SCENE_H_
#define __CORE_SCENE_H_

#include "Camera.h"
#include "Light.h"
#include "Mesh.h"

#include "Core/Container/Vector.h"
#include "Core/Math/Transform.h"

namespace Influx::Scene
{
	class Scene final
	{
	public:
		void AddCamera(const Camera& camera)
		{
			m_cameras.push_back(camera);
		}

		void AddLight(const Light& light)
		{
			m_lights.push_back(light);
		}

		void AddMesh(const Mesh& mesh)
		{
			m_meshes.push_back(mesh);
		}

		const Vector<Camera>& GetCameras() const
		{
			return m_cameras;
		}

		const Vector<Light>& GetLights() const
		{
			return m_lights;
		}

		const Vector<Mesh>& GetMeshes() const
		{
			return m_meshes;
		}

	private:
		Vector<Camera> m_cameras;
		Vector<Light> m_lights;
		Vector<Mesh> m_meshes;

		Math::TransformGraph m_transformGraph;
	};
}

#endif