#pragma once

#include "threads.h"
#include "application/scene/scene.h"

#include "Core/Container/Vector.h"

namespace influx::application
{
	class gamethread final : public dedicated_thread
	{
	public:
		virtual void initialize() override;
		virtual void tick() override;
		virtual void cleanup() override;

		virtual e_dedicated_thread get_thread_type() const override
		{
			return e_dedicated_thread::gamethread;
		}

	private:
		vector<entity> m_entities{};
		entity m_camera_entity{};
	};
}