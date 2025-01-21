#pragma once
#include "rgcommon.h"

namespace influx::rendergraph
{
	class rgchild
	{
		friend class rendergraph;
		friend class rgpass_builder;

	public:
		inline bool is_imported() const
		{
			return m_is_imported;
		}

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
		texture_desc m_desc;
	};

	class rgbuffer final : public rgchild
	{
		friend class rendergraph;

	private:
		rgbuffer() = default;

		rgbuffer_id m_id;
		buffer_desc m_desc;
	};
}