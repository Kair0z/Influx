#include "graphics_pch.h"

#include "influx_graphics/queue.h"
#include "influx_graphics/commandlist.h"

namespace influx::graphics
{
	queue::queue(const queue_desc& desc)
		: m_desc{ desc }
	{
	}

	void queue::submit(const vector<commandlist*>& commandlists)
	{
		submit_commandlists(commandlists);
		post_submit(commandlists);
	}

	void queue::post_submit(const vector<commandlist*>& commandlists)
	{
		for (commandlist* list : commandlists)
		{
			list->post_submit(this);
		}
	}
}

