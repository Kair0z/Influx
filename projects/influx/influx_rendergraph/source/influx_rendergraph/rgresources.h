#pragma once
#include "rgcommon.h"

namespace influx::rendergraph
{
	class rgchild
	{
		friend class rendergraph;

	protected:
		rgchild() = default;

	private:
		rgpass* m_writer;
		rgpass* m_last_user;
		uint32 m_refcount;
		graphics::resource* m_resource;
		bool m_is_imported;
	};

	class rgtexture final : public rgchild
	{
		friend class rendergraph;

	private:
		rgtexture() = default;

		rgtexture_id m_id;
		bool m_is_imported;
	};

	class rgbuffer final : public rgchild
	{
		friend class rendergraph;

	private:
		rgbuffer() = default;

		rgbuffer_id m_id;
		bool m_is_imported;
	};
}