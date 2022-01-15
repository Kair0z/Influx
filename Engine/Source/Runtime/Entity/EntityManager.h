#pragma once

#ifndef _ENTITY_MANAGER_H_
#define _ENTITY_MANAGER_H_

#include "ENTT/entt.hpp"

#include "Core/Memory/Reference.h"
#include "Core/Container/Containers.h"
#include "Core/Assert/Assert.h"

namespace Influx
{
	class Entity;

	/* Wrapper around entt::registry */
	class Registry final
	{
	public:
		Entity* CreateEntity();
		entt::registry& GetENTTRegistry();
		const List<Entity*>& GetEntities() const;

	private:
		entt::registry mRegistry{};
		List<Entity*> mpEntities{};
	};

	class Entity final
	{
	public:
		using ID = entt::entity;
		virtual ~Entity() = default;

		template <class C, class... Args>
		inline Ptr<C> AddComponent(Args&&... args)
		{
			ASSERT(!HasComponent<C>());
			return &mpRegistryRef->GetENTTRegistry().emplace<C>(mID, std::forward<Args>(args)...);
		}

		template <class C>
		inline Ptr<C> GetComponent() 
		{
			ASSERT(HasComponent<C>());
			return &mpRegistryRef->GetENTTRegistry().get<C>(mID);
		}

		template <class C>
		inline bool HasComponent() const
		{
			//mpRegistryRef->GetENTTRegistry().get<C>(mID);
			return false;
		}

		template <class C>
		inline void RemoveComponent()
		{
			ASSERT(HasComponent<C>());
			mpRegistryRef->GetENTTRegistry().remove_if_exists<C>(mID);
		}

		/* [Noimpl] */
		inline void RemoveAllComponents()
		{
			//mpRegistryRef->GetENTTRegistry().remove_all(mID);
		}

		inline ID GetID() const { return mID; }

	private:
		ID mID;
		Ptr<Registry> mpRegistryRef;

		/* Registry only creates and destroys Entity */
		friend class Registry;
		Entity() = default;
	};

	class EntityManager final
	{
	public:
		static Ptr<Registry> CreateRegistry();

	private:
		static Vector<Registry*> mRegistries;
	};
}

#endif

