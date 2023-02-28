
#include <vector>

class IComponent
{
	virtual void Update(float elapsedSeconds) = 0;
	virtual void Draw() const = 0;

	virtual ~IComponent() = default;
};

class RenderComponent final : public IComponent
{
	virtual void Update(float elapsedSeconds)
	{

	}

	virtual void Draw() const
	{

	}
};

class MegaBeastComponent final : public IComponent
{
	virtual void Update(float elapsedSeconds) override
	{

	}

	virtual void Draw() const override
	{

	}

private:
	struct MegaBeastTexture
	{
		struct MegaBeastPixel
		{
			float a, b, c, d;
		};

		MegaBeastPixel pixels[4096 * 4096];
	};

	MegaBeastTexture m_texture;
};

struct GameObject final
{
public:
	void AddComponent(IComponent* component)
	{
		mp_components.push_back(component);
	}

	void RemoveComponent(IComponent* component)
	{

	}

	void Update(float elapsedSeconds)
	{

	}

	void Render() const
	{

	}

	const std::vector<IComponent*>& GetComponents() const
	{
		return mp_components;
	}

	virtual ~GameObject()
	{
		for (IComponent* componentPtr : mp_components)
		{
			delete componentPtr;
			componentPtr = nullptr;
		}
	}

private:
	std::vector<IComponent*> mp_components;
};

struct Scene
{
	void AddGameObject(GameObject object)
	{
		m_gameObjects.push_back(object);
	}

	std::vector<GameObject> m_gameObjects;
};

#include <iostream>

int main()
{
	{
		Scene scene{};

		constexpr int numGameObjects = 10;

		// Load
		{
			for (int i = 0; i < numGameObjects; ++i)
			{
				scene.AddGameObject(GameObject{});

				scene.m_gameObjects.back().AddComponent(new MegaBeastComponent());
			}
		}

		for (int i = 0; i < scene.m_gameObjects.size(); ++i)
		{
			// Update
			scene.m_gameObjects[i].Update(0.0f);
			scene.m_gameObjects[i].Render();
		}
	}
	
	std::cin.get();
}