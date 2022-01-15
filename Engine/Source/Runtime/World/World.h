#pragma once
#include "Core/Container/Containers.h"
#include "Core/Memory/Reference.h"
#include "Core/Type/String.h"

namespace Influx
{
	/* This will manage the in game Objects */
	/* World only keeps track of what objects' relations are to eachother */
	/* Creation/Destruction happens externally in Object-related classes */

	class Registry;
	class GameObject;

	class World final
	{
	public:
		static Ptr<World> Create();
		~World();

		/* Creation of World data (Not game) */
		void Initialize();

		void Start();
		void Update();
		void FixedUpdate();
		void Render();

		GameObject* NewGameObject(const String& name);
		const List<GameObject*>& GetGameObjects() const;

	private:
		World() = default;

		List<GameObject*> mpGameObjects{};
		Ptr<Registry> mpEntityRegistry;
	};
}


