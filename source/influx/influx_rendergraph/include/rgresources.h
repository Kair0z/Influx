#pragma once
#include "rgcommon.h"

namespace influx::rendergraph
{
	class rgpass;

	struct rgtexture_info final
	{
		string m_name;
	};

	struct rgbuffer_info final
	{
		string m_name;
	};

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

		/* only resets info relating to graph usage */
		inline void reset_graph()
		{
			m_writer = nullptr;
			m_final_pass = nullptr;
			m_refcount = 0u;
		}

	protected:
		rgchild() = default;

		rgpass* m_writer;
		rgpass* m_final_pass;
		uint32	m_refcount;

		rhi_resource* m_resource;
		bool m_is_imported;
		rgname m_name;
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