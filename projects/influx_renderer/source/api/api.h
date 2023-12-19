#pragma once

namespace influx::renderer::api
{
	class base_object
	{
	public:
		inline void* get_native() const
		{
			return mp_native;
		}

		inline void*& get_native()
		{
			return mp_native;
		}

	protected:
		void* mp_native = nullptr;
	};

#pragma region logic device
	class logical_device final : public base_object
	{
	public:
		logical_device();
	};

#pragma endregion

#pragma region command queue
	enum class e_command_queue_type
	{
		graphics,
		copy,
		compute,
		max
	};

	enum class e_command_queue_flags
	{

	};

	struct command_queue_desc
	{
		e_command_queue_type m_type;
		e_command_queue_flags m_flags;
		float m_priority;
	};

	class command_queue final : public base_object
	{
	public:
		command_queue(const logical_device& device, const command_queue_desc& desc);
	};
#pragma endregion

}