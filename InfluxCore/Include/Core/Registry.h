#pragma once

#ifndef __CORE_REGISTRY_H_
#define __CORE_REGISTRY_H_

#include <memory>

#define __CORE_REGISTRY_USE_CORE 1
#if __CORE_REGISTRY_USE_CORE
#include "Core/Container/Array.h"
#include "Core/Container/Vector.h"
#include "Core/BasicTypes.h"
#else
#endif

namespace Influx
{
	namespace Internal
	{
		class IRegistry
		{

		};
	}

	template <class _B, class _Enum>
	class Registry final : public Internal::IRegistry
	{
	private:
		using BaseClass = _B;
		using Enum = _Enum;

		constexpr static uint64 k_EnumMaxValue = static_cast<uint64>(Enum::Max);

		struct Child final
		{
			BaseClass* pObjectPointer;
		};

		using Container = Vector<Child>;
		
	public:
		template <Enum _E>
		bool CanRegisterObject() const
		{
			return true;
		}

		template <Enum _E>
		BaseClass* CreateObject()
		{
			Child newChild{};

			newChild.pObjectPointer = new BaseClass();

			constexpr uint64 index = static_cast<uint64>(_E);
			mp_childMap[index].push_back(newChild);
		}

		template <Enum _E>
		const Container& GetObjectsOfType() const
		{
			constexpr uint64 index = static_cast<uint64>(_E);
			return mp_childMap[index];
		}

		template <Enum _E>
		uint64 GetNumObjectsOfType()
		{
			return GetObjectsOfType().size();
		}

		template <Enum _E>
		bool HasObjectOfType() const
		{
			return GetNumObjectsOfType() != 0u;
		}

	private:
		Array<Container, k_EnumMaxValue> mp_childMap;
	};
}

#endif