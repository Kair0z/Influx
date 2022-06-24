#pragma once

#include "Core/Memory/Reference.h"
#include "RHITypes.h"

namespace Influx
{
#pragma region ForwardDeclarations
	class RHIGraphicsCommandList;
	class RenderAPI;
	struct CommandQueueDesc;
#pragma endregion

	class RHICommandQueue
	{
	public:
		static Ptr<RHICommandQueue> Create(const Ptr<RenderAPI> api, const CommandQueueDesc& desc);

		virtual Ptr<RHIGraphicsCommandList> GetNewGraphicsCommandList(const Ptr<RenderAPI> api) = 0;
		virtual void ExecuteCommandList(Ptr<RHIGraphicsCommandList> list) = 0;
		virtual void Flush() = 0;

		virtual ~RHICommandQueue() = default;
	};

	struct CommandQueueDesc
	{
		ECommandQueueType type;
	};
}


