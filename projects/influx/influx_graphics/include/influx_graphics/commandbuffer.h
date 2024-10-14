#pragma once

#include "commandlist.h"
#include "commandallocator.h"

namespace influx::graphics
{
	class commandlist;
	class command_allocator;
	class queue;
}

namespace influx::graphics
{
	namespace detail
	{
		class command_base
		{
		public:
			virtual void* get_raw_data() = 0u;
		};

		template <typename _data>
		class tcommand : public detail::command_base
		{
		public:
			tcommand(const _data& data) : m_data{ data } {}

			virtual void* get_raw_data() override
			{
				return &get_data();
			}

			_data& get_data()
			{
				return m_data;
			}

		private:
			_data m_data{};
		};
	}

#pragma region commands
	struct cmd_begin_renderpass final
	{
		cmd_begin_renderpass() = default;
	};

	struct cmd_end_renderpass final
	{
		cmd_end_renderpass() = default;
	};

	struct cmd_draw_instanced final
	{
		cmd_draw_instanced() = default;
		cmd_draw_instanced(
			uint32 num_vertices_per_instance,
			uint32 num_instances,
			uint32 start_vertex,
			uint32 start_instance)
			: m_num_vertices_per_instance(num_vertices_per_instance)
			, m_num_instances(num_instances)
			, m_start_vertex(start_vertex)
			, m_start_instance(start_instance) {}

		uint32 m_num_vertices_per_instance = 1u;
		uint32 m_num_instances = 1u;
		uint32 m_start_vertex = 0;
		uint32 m_start_instance = 0;
	};
#pragma endregion

	// a encapsulating layer that manages underlying command allocator and GPU state
	class commandbuffer : public base
	{
	public:
		enum class e_state : uint8
		{
			idle,
			submitted,
			finished,
			count
		};

		virtual void submit() = 0;
		virtual void submit(queue* queue) = 0;

		virtual e_state get_state() const = 0;

		bool is_finished_gpu() const
		{
			return get_state() == e_state::finished;
		}

		template <typename _command_data, typename... _args> requires std::is_constructible_v<_command_data, _args...>
		void push(_args&&... args)
		{
			detail::tcommand<_command_data>* new_command = new detail::tcommand<_command_data>(_command_data(std::forward<_args>(args)...));
			m_commands.emplace_back(new_command);
		}

	private:
		graphics::commandlist* m_commandlist;
		graphics::command_allocator* m_allocator;
		
	protected:
		e_state m_state;
		vector<detail::command_base*> m_commands;
	};
}