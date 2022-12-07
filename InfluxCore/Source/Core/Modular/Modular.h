#pragma once

#ifndef _CORE_MODULAR_H_
#define _CORE_MODULAR_H_

#include "Core/Math/Matrix.h"

namespace Influx
{
	class IModule
	{

	};

	// Adding Modules
	// - Set 'activestate', defer allocating data...
	// - Dependencies... 

	template <size_t _C>
	class IModuleRegister final
	{
		using matrix_type = Math::Matrix<uint8_t, _C, _C>;
		matrix_type m_register;

	public:
		void AddModule();
		void RemoveModule();
		void HasModule();

	private:
	};
}

#endif