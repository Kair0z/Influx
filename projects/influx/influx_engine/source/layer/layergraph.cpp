#include "engine_pch.h"
#include "layer/layergraph.h"

namespace influx::engine
{
	void layergraph::update(const update_context& ctx)
	{
		m_graph.traverse([&ctx](const graph_node& node)
		{
			layer* layer = node.m_data;
			if (layer == nullptr) return;

			if (layer->m_frame_counter == 0u)
			{
				layer->on_start();
			}
			else
			{
				layer->on_update(ctx);
			}

			layer->m_frame_counter++;
		});
	}

	void layergraph::on_keydown(input::e_key key)
	{
		m_graph.traverse([key](const graph_node& node)
		{
			layer* layer = node.m_data;
			if (layer == nullptr) return;

			layer->on_keydown(key);
		});
	}

	void layergraph::on_keyup(input::e_key key)
	{
		m_graph.traverse([key](const graph_node& node)
		{
			layer* layer = node.m_data;
			if (layer == nullptr) return;

			layer->on_keyup(key);
		});
	}

	void layergraph::on_ascii_down(char ascii)
	{
		m_graph.traverse([ascii](const graph_node& node)
		{
			layer* layer = node.m_data;
			if (layer == nullptr) return;

			layer->on_ascii_down(ascii);
		});
	}

	void layergraph::on_ascii_up(char ascii)
	{
		m_graph.traverse([ascii](const graph_node& node)
		{
			layer* layer = node.m_data;
			if (layer == nullptr) return;

			layer->on_ascii_up(ascii);
		});
	}

	void layergraph::on_mouse_move(const input::mouse_position& position)
	{
		m_graph.traverse([position](const graph_node& node)
		{
			layer* layer = node.m_data;
			if (layer == nullptr) return;

			layer->on_mouse_move(position);
		});
	}

	void layergraph::on_mouse_down(input::e_mouse_button button, const input::mouse_position& position)
	{
		m_graph.traverse([button, position](const graph_node& node)
		{
			layer* layer = node.m_data;
			if (layer == nullptr) return;

			layer->on_mouse_down(button, position);
		});
	}

	void layergraph::on_mouse_up(input::e_mouse_button button, const input::mouse_position& position)
	{
		m_graph.traverse([button, position](const graph_node& node)
		{
			layer* layer = node.m_data;
			if (layer == nullptr) return;

			layer->on_mouse_up(button, position);
		});
	}
}