#pragma once

#include "Core/Memory/Reference.h"

#include "Core/Container/Containers.h"
#include "Core/Function/Function.h"

namespace Influx
{
	class World;
	class RHIGraphicsCommandList;

	/* A capture of Gamestate for the renderer to bring to the screen */
	class RenderFrame final
	{
	public:
		enum class RenderPass
		{
			PreRender,
			Render,
			PostRender,
			EditorRender,
			END
		};

		static Ptr<RenderFrame> Create(const Ptr<World> world);
		~RenderFrame() = default;

		using Command = Function<void(Ptr<RHIGraphicsCommandList> cmdList)>;
		void EnqueueRenderCommand(Command cmd, RenderPass pass);

		const Vector<Command>& GetCommands(RenderPass pass) const;

	private:
		Vector<Vector<Command>> mCommandQueues;

		RenderFrame() = default;
	};
}


