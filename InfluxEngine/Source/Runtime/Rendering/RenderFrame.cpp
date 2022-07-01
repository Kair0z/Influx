#include "pch.h"
#include "RenderFrame.h"

namespace Influx
{
	Ptr<RenderFrame> RenderFrame::Create(const Ptr<World> world)
	{
		RenderFrame* newFrame = new RenderFrame();
		
		for (uint32_t i{}; i < (uint32_t)RenderFrame::RenderPass::END; ++i)
		{
			newFrame->mCommandQueues.push_back(Vector<RenderFrame::Command>());
		}

		return newFrame;
	}

	void RenderFrame::EnqueueRenderCommand(Command cmd, RenderPass pass)
	{
		mCommandQueues[(size_t)pass].push_back(cmd);
	}

	const Vector<RenderFrame::Command>& RenderFrame::GetCommands(RenderPass pass) const
	{
		return mCommandQueues[(size_t)pass];
	}
}

