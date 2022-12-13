#pragma once

#ifndef _CORE_MODULAR_H_
#define _CORE_MODULAR_H_

#include "Core/Math/Matrix.h"

namespace Influx
{
	class IModule
	{

	};

	// Thoughts on modules:
	// - Can we assume Modules should always be unique?
	// - Dependencies???
	// - Set 'activestate', defer allocating data...

	template <size_t _C>
	class IModuleRegister final
	{
		using matrix_type = Math::Matrix<uint8_t, _C, _C>;
		matrix_type m_register;

	public:
		void AddModule();
		void RemoveModule();
		void HasModule();

		void GetModuleDependencies();

	private:
	};
}

#endif