#pragma once
#include "Core/Container/Map.h"
#include "Core/Type/String.h"
#include "Core/Memory/Reference.h"

namespace Influx
{
	class AssetManager final
	{
		using AssetID = uint64_t;

	public:
		static Ptr<AssetManager> Create();

		template <typename T>
		inline void Load(const char* filepath)
		{

		}

	private:
		AssetManager() = default;
	};
}


