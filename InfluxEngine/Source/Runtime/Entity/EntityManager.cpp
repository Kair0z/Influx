#include "pch.h"
#include "EntityManager.h"

namespace Influx
{
    Vector<Registry*> EntityManager::mRegistries{};

    entt::registry& Registry::GetENTTRegistry()
    {
        return mRegistry;
    }

    const List<Entity*>& Registry::GetEntities() const
    { 
        return mpEntities;
    }

    Ptr<Registry> EntityManager::CreateRegistry()
    {
        Ptr<Registry> newRegistry = new Registry();
        mRegistries.push_back(newRegistry);

        return newRegistry;
    }

    Entity* Registry::CreateEntity()
    {
        Entity* newEntity = new Entity();
        mpEntities.push_back(newEntity);
        newEntity->mID = mRegistry.create();
        newEntity->mpRegistryRef = this;

        return newEntity;
    }
}

