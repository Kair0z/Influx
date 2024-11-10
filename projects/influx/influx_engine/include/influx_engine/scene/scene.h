#pragma once

// influx::core
#include "core/container/vector.h"

namespace influx::engine
{
	class actor
	{

	};

	class scene
	{
	public:
		scene() = default;

		static scene* load_from_file(const string& path);

		const vector<actor>& get_all_actors() const;

	private:
		vector<actor> m_actors{};
	};
}