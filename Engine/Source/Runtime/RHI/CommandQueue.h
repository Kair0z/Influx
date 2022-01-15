#pragma once

#include "Core/Memory/Reference.h"
#include "RHITypes.h"

namespace Influx
{
#pragma region ForwardDeclarations
	class RHIGraphicsCommandList;
	class RenderAPI;
#pragma endregion

	struct CommandQueueDesc;
	class RHICommandQueue
	{
	public:
		static Ptr<RHICommandQueue> Create(const Ptr<RenderAPI> api, const CommandQueueDesc& desc);

		virtual Ptr<RHIGraphicsCommandList> GetNewGraphicsCommandList(const Ptr<RenderAPI> api) = 0;
		virtual void ExecuteCommandList(Ptr<RHIGraphicsCommandList> list) = 0;
		virtual void Flush() = 0;
	};

	struct CommandQueueDesc
	{
		ECommandQueueType type;
	};
}


