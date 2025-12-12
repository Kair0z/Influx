#pragma once

#include "cereal/types/string.hpp"
#include "cereal/archives/json.hpp"
#include "cereal/archives/binary.hpp"

// influx::core
#include "core/math/matrix.h"

// these custom serialization functions need to be in the cereal namespace for cereal to access them!
namespace cereal
{
	// influx::math::matrix4x4f
	template <class _archive> void serialize(_archive& arch, influx::math::matrix4x4f& matrix)
	{
		arch(matrix.m_data);
	}

	// influx::string
	template <typename _archive> void serialize(_archive& arch, influx::string& str)
	{
		std::string std_cpy = str.get_std();
		arch(std_cpy);
		str = std_cpy;
	}
}