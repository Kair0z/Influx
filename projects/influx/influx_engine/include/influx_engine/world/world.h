#pragma once

// influx::core
#include "core/container/pool.h"
#include "core/container/list.h"
#include "core/container/vector.h"
#include "core/pointer.h"

// influx::engine
#include "influx_engine/component.h"

namespace influx::engine
{
	class scene;
}

namespace influx::renderer
{
	struct scene;
	struct scene2D;
}

namespace influx::engine
{
	using entity = uint64;

	class world final
	{
		template <uint32 _num>
		using tentity_pool = pool<entity, _num>;
		using entity_pool_8K = tentity_pool<8 * 1024u>;
		using entity_pool_4K = tentity_pool<4 * 1024u>;
		using entity_pool_1K = tentity_pool<1 * 1024u>;
		using entity_pool = entity_pool_4K;

	public:
		world();
		virtual ~world();

#if 0
		// -- layer-end -
		uint32 create_entity()
		{
			return (uint32)m_registry.create();
		}

		template <class _ctype, class ..._args>
		_ctype* create_component(uint32 entt, _args&&... args)
		{
			return &m_registry.emplace_or_replace<_ctype>((entt::entity)entt, args...);
		}
#endif

		// -- engine-end
		void build_renderscene(renderer::scene&, renderer::scene2D&) const;

		// deletes unreferenced entities
		void flush();
		
	};
}