#pragma once
#include "influx_graphics/base.h"

namespace influx::graphics
{
	struct rootsignature_desc final
	{

	};

	class rootsignature : public base
	{
	public:
		
	protected:
		rootsignature(const rootsignature_desc& desc)
			: m_desc{desc}
		{

		}

	private:
		rootsignature_desc m_desc;
	};
}