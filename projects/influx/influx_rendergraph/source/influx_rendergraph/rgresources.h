#pragma once
#include "rgcommon.h"

namespace influx::rendergraph
{
	class rgchild
	{
	protected:
		rgchild() = default;

	private:

	};

	class rgtexture final : public rgchild
	{
		friend class rendergraph;

	private:
		rgtexture() = default;

		rgtexture_id m_id;
		bool m_is_imported;
		graphics::resource* m_resource;
	};

	class rgbuffer final : public rgchild
	{
		friend class rendergraph;

	private:
		rgbuffer() = default;

		rgbuffer_id m_id;
		bool m_is_imported;
		graphics::resource* m_resource;
	};
}