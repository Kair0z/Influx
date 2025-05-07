#pragma once
#include "rgcommon.h"

namespace influx::rendergraph
{
	class rgchild
	{
		friend class rendergraph;
		friend class rgpass_builder;
		friend class rgpass_context;

	public:
		inline bool is_imported() const
		{
			return m_is_imported;
		}

		inline void reset()
		{
			m_writer = nullptr;
			m_last_user = nullptr;
			m_refcount = 0u;
		}

	protected:
		rgchild() = default;

		rgpass* m_writer;
		rgpass* m_last_user;
		uint32 m_refcount;
		graphics::resource* m_resource;
		bool m_is_imported;
	};

	class rgtexture final : public rgchild
	{
		friend class rendergraph;
		friend class rgpass_builder;

	private:
		rgtexture() = default;

		rgtexture_id m_id;
		texture_desc m_desc;
	};

	class rgbuffer final : public rgchild
	{
		friend class rendergraph;
		friend class rgpass_builder;

	private:
		rgbuffer() = default;

		rgbuffer_id m_id;
		buffer_desc m_desc;
	};
}