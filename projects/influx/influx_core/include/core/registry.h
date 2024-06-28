#pragma once

#ifndef __CORE_REGISTRY_H_
#define __CORE_REGISTRY_H_

#define __CORE_REGISTRY_USE_CORE 1

#if __CORE_REGISTRY_USE_CORE
#include "core/container/array.h"
#include "core/container/vector.h"
#include "core/basetypes.h"
#else
#endif

#include <memory>

namespace influx
{
	namespace detail
	{
		class i_registry
		{

		};
	}

	template <class _B, class _Enum>
	class registry final : public detail::i_registry
	{
	private:
		using base_class = _B;

		constexpr static uint64 k_enum_max = static_cast<uint64>(_Enum::maximum);

		struct Child final
		{
			base_class* pObjectPointer;
		};

		using Container = vector<Child>;
		
	public:
		template <_Enum _E>
		bool CanRegisterObject() const
		{
			return true;
		}

		template <_Enum _E>
		base_class* CreateObject()
		{
			Child newChild{};

			newChild.pObjectPointer = new base_class();

			constexpr uint64 index = static_cast<uint64>(_E);
			mp_childMap[index].push_back(newChild);
		}

		template <_Enum _E>
		const Container& GetObjectsOfType() const
		{
			constexpr uint64 index = static_cast<uint64>(_E);
			return mp_childMap[index];
		}

		template <_Enum _E>
		uint64 GetNumObjectsOfType()
		{
			return GetObjectsOfType().dimension();
		}

		template <_Enum _E>
		bool HasObjectOfType() const
		{
			return GetNumObjectsOfType() != 0u;
		}

	private:
		Array<Container, k_EnumMaxValue> mp_childMap;
	};
}

#endif