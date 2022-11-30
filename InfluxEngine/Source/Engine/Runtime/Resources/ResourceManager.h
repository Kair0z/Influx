#pragma once

namespace Influx
{
	class ResourceManager final
	{
		using ID = uint64_t;

	public:
		static Ptr<ResourceManager> Create();

	private:
		ResourceManager() = default;
	};
}


