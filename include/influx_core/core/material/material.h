#pragma once
#include "core/baseetypes.h"

namespace influx
{
	enum class e_material_property_type : uint8
	{
		scalar,
		vector,
		count
	};

	class material_property
	{
	public:

	private:
		e_material_property_type m_type;
	};

	class material final
	{
	public:

	private:

	};
}