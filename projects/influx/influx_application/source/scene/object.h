#pragma once

namespace influx::scene
{
	using actor_id = uint64;
	using comp_id = uint64;
	using obj_id = uint64;

	constexpr static uint64 k_invalid_id = (uint64)-1;

	class object
	{
	public:
		obj_id get_id() const;

		virtual ~object() = default;

	private:
		obj_id m_id = k_invalid_id;
	};
}