#pragma once
#include "Runtime/Components/Component.h"
#include "Core/Type/String.h"

namespace Influx
{
	class TagComponent final : public Component
	{
	public:
		TagComponent(const String& tag) : mTag{ tag }{}

	private:
		String mTag{};
	};
}


