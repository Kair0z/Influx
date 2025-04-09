#pragma once

namespace influx::graphics
{
	enum class e_hitgroup_type
	{
		triangles,
		count
	};

	struct hitgroup
	{
	public:
		e_hitgroup_type m_type;
	};
}