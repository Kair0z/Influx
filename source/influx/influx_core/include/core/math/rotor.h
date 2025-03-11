#pragma once
#include "vector.h"

// https://www.youtube.com/watch?v=4m3vaC1b5qU
namespace influx::math
{
	using rotdim = uint32;

	template <typename _t, rotdim _d>
	class rotor final
	{
		static constexpr rotdim k_dimension = _d;
		static_assert(k_dimension > 1u, "rotors are not useable in a space without planes!");

		// a multivec (bivector / trivector)
		// represents an orientation and scale
		// in 2D space, there's only 1 plane of rotation
		// so there's only 1 multivector component
		// in 3D space, there's suddenly 3 planes of rotation
		// so there's 3 multivector components
		static constexpr rotdim k_num_bivectors = _d * (_d - 1) / 2;
		using multivec = vector<_t, k_num_bivectors>;

		_t			m_scalar{};
		multivec	m_multivector{};
		
		using value_type = _t;

		rotor() = default;
		rotor(_t scalar, const multivec& multivec)
			: m_scalar{ scalar }, m_multivector{ multivec }{}


	};
}