#include "graphics_pch.h"

#include "influx_graphics/queue.h"
#include "influx_graphics/commandlist.h"

namespace influx::graphics
{
	queue::queue(const queue_desc& desc)
		: m_desc{ desc }
	{
	}

	result<> queue::submit(const vector<commandlist*>& commandlists)
	{
		result<> res = {};
		
		res = submit_commandlists(commandlists);
		if (!res) return result<>::make_error("error: queue failed submitting commandlists.");

		res = post_submit(commandlists);
		if (!res) return result<>::make_error("error: queue post_submit failed.");

		return res;
	}

	result<> queue::post_submit(const vector<commandlist*>& commandlists)
	{
		result<> res = {};
		for (commandlist* list : commandlists)
		{
			res = list->post_submit(this);
		}
		return res;
	}
}

