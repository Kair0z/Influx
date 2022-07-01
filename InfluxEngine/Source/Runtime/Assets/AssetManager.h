#pragma once

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


