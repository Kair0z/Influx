#pragma once

#include "Core/String.h"
#include "Core/Pointer.h"
#include "Core/Container/Vector.h"

namespace Influx::Renderer
{
	class Sink;
	class Source;

	class Pass final
	{
		using SinkPtr	= Ptr<Sink>;
		using SrcPtr	= Ptr<Source>;

	public:
		Pass() = default;

		void Execute();

		void RegisterSink(SinkPtr pSink);
		void RegisterSource(SrcPtr pSource);

	private:
		String m_name;

		Vector<SinkPtr> mp_sinks;
		Vector<SrcPtr> mp_sources;
	};
}


