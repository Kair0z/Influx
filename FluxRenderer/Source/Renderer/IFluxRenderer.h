#pragma once

#include "Core/Math/Vector.h"
#include "Core/Scene/Mesh.h"
#include "Core/Scene/Camera.h"
#include "Core/Container/Vector.h"

#include "Core/Platform/Platform.h"

namespace Influx
{
	class IFluxRenderer
	{
	protected:
		IFluxRenderer() = default;

		enum class ESwapchainBuffering : uint8
		{
			Single = 1u,
			Double = 2u,
			Triple = 3u,
			Max
		};
		constexpr static ESwapchainBuffering k_swapchainBuffering = ESwapchainBuffering::Triple;
		constexpr static uint8 k_numSwapchainBuffers = static_cast<uint8>(k_swapchainBuffering);


	public:
		struct MaterialData final
		{
			MaterialData() = default;
			MaterialData(const Vector<byte>& vs, const Vector<byte>& ps) 
				: VertexShader{ vs }, PixelShader{ ps } {}

			Vector<byte> VertexShader;
			Vector<byte> PixelShader;
		};

		virtual void BuildRenderWork(Platform::WindowHandle windowHandle) = 0;

		virtual void PresentToWindow(Platform::WindowHandle windowHandle) = 0;

		void SetMaterial(const MaterialData& material)
		{
			m_material = material;
		}

		void AddMesh(const Scene::Mesh& mesh)
		{
			m_meshes.push_back(mesh);

			m_numVertices = 0u;
			m_numIndices = 0u;

			for (const Scene::Mesh& mesh : m_meshes)
			{
				m_numVertices += mesh.GetVertices().size();
				m_numIndices += mesh.GetIndices().size();
			}

			m_vertexBufferSize = m_numVertices * GetVertexSize();
			m_indexBufferSize = m_numIndices * GetIndexSize();
		}
		
		void SetCameraTransform(const Math::Vectorf3& position, const Math::Vectorf3& forward)
		{
			m_cameraPosition = position;
			m_cameraForward = forward;
		}

		void SetCameraData(const Scene::Camera& cameraData)
		{
			m_cameraData = cameraData;
		}

		const Vector<Scene::Mesh>& GetMeshes() const
		{
			return m_meshes;
		}

		const Scene::Camera& GetCameraData() const
		{
			return m_cameraData;
		}

		const Math::Vectorf3& GetCameraPosition() const
		{
			return m_cameraPosition;
		}

		const Math::Vectorf3& GetCameraForward() const
		{
			return m_cameraForward;
		}

		const MaterialData& GetMaterial() const
		{
			return m_material;
		}

	private:
		Scene::Camera m_cameraData;
		Math::Vectorf3 m_cameraPosition;
		Math::Vectorf3 m_cameraForward;
		MaterialData m_material;

		uint64 m_numVertices;
		uint64 m_numIndices;
		uint64 m_vertexBufferSize;
		uint64 m_indexBufferSize;

		Vector<Scene::Mesh> m_meshes;

	protected:
		constexpr static uint64 GetVertexSize() { return sizeof(Scene::Mesh::Vertex); }
		constexpr static uint64 GetIndexSize() { return sizeof(Scene::Mesh::Index); }

		uint64 GetIndexBufferSize() const
		{
			return m_indexBufferSize;
		}

		uint64 GetVertexBufferSize() const
		{
			return m_vertexBufferSize;
		}
	};
}


