#pragma once

#include "core/basetypes.h"
#include "core/math/vector.h"
#include "core/container/vector.h"
#include "core/math/matrix.h"

namespace influx::graphics
{
	enum class e_hitgroup_type
	{
		triangles,
		count
	};

	struct hitgroup final
	{
	public:
		e_hitgroup_type m_type;
	};

	enum class e_acceleration_structure_type : uint8
	{
		bottom,
		top,
		count
	};

	class tlas_resources final
	{
	public:
		resource* m_instances_buffer = nullptr;
		resource* m_scratch_buffer = nullptr;
		resource* m_tlas_buffer = nullptr;
	};

	class blas_resources final
	{
	public:
		// created vertex / index buffers
		// these are in cpu-writable state!
		resource* m_vertex_buffer = nullptr;
		resource* m_index_buffer = nullptr;
		
		resource* m_scratch_buffer = nullptr;
		resource* m_blas_buffer = nullptr;
	};

	struct blas_update_args final
	{
		using vertex = math::float3;
		using index = uint16;
		vector<vertex> m_vertices{};
		vector<index> m_indices{};
	};

	struct tlas_update_args final
	{
		using instance = math::matrix4x4f;
		vector<instance> m_instances{};
		blas_resources* m_blas = nullptr;
	};

	struct blas_create_args final
	{
		blas_update_args m_worst_case_update{};
	};

	struct tlas_create_args final
	{
		
	};

	template <e_acceleration_structure_type _t>
	using as_create_args = std::tuple_element_t<static_cast<uint64>(_t), std::tuple<
		blas_create_args,
		tlas_create_args>>;

	template <e_acceleration_structure_type _t>
	using as_resources = std::tuple_element_t<static_cast<uint64>(_t), std::tuple<
		blas_resources,
		tlas_resources>>;

	template <e_acceleration_structure_type _t>
	using as_update_args = std::tuple_element_t<static_cast<uint64>(_t), std::tuple<
		blas_update_args,
		tlas_update_args>>;
}