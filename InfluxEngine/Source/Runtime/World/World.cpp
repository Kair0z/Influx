#include "pch.h"
#include "World.h"
#include "Runtime/World/GameObject.h"
#include "Runtime/Entity/EntityManager.h"

#include "Runtime/Components/TransformComponent.h"
#include "Runtime/Components/RenderComponent.h"
#include "Runtime/Components/TagComponent.h"

namespace Influx
{
	Ptr<World> World::Create()
	{
		Ptr<World> newWorld = new World();

		/* Create entityRegistry */
		newWorld->mpEntityRegistry = EntityManager::CreateRegistry();

		return newWorld;
	}

	GameObject* World::NewGameObject(const String& name)
	{
		GameObject* newObject = new GameObject();
		newObject->mpEntity = mpEntityRegistry->CreateEntity();

		/* Add tag component... */
		newObject->mpEntity->AddComponent<TagComponent>("Object");

		/* Add Transform Component... */
		newObject->mpEntity->AddComponent<TransformComponent>(Vector3f{}, Vector3f{}, Vector3f{1.0f, 1.0f, 1.0f});
		return newObject;
	}

	void World::Initialize()
	{
		using namespace Math;
		for (uint32_t i = 0; i < 100; ++i)
		{
			GameObject* obj = NewGameObject("Empty");
			obj->mpEntity->AddComponent<RenderComponent>();
		}
	}


	void World::Start()
	{
		NewGameObject("GameObject");
	}

	void World::Update()
	{
		auto view = mpEntityRegistry->GetENTTRegistry().view<const TransformComponent>();
	}

	void World::FixedUpdate()
	{
	}

	void World::Render()
	{
		auto view = mpEntityRegistry->GetENTTRegistry().view<const TransformComponent, const RenderComponent>();
		for (auto [e, transform, render] : view.each())
		{
			
		}
	}

	

	const List<GameObject*>& World::GetGameObjects() const
	{
		return mpGameObjects;
	}

	World::~World()
	{
		for (GameObject* obj : mpGameObjects)
		{
			delete obj;
			obj = nullptr;
		}

		delete mpEntityRegistry;
		mpEntityRegistry = nullptr;
	}
}

