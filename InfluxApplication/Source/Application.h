#pragma once

#include "../ImGui/imgui.h"

#include "Core/String.h"
#include "Core/Math/Vector.h"

namespace Influx
{
	enum class EApplicationType
	{
		Default
	};

	struct ApplicationDescription
	{
		EApplicationType Type			= EApplicationType::Default;
		String Name						= "InfluxApp";
		Math::Vectoru2 InitWindowSize	= { 640u, 480u };
	};

	class Application final
	{
		using OnUIRenderCallback = void (*)();

	public:
		Application(int argc, char** argv, const ApplicationDescription& desc);
		
		void Run(OnUIRenderCallback onUIRender);
		void Quit();

		Application(const Application&) = delete;
		Application(Application&&) = delete;
		Application& operator=(const Application&) = delete;
		Application& operator=(Application&&) = delete;
		~Application();

	private:
		bool m_hasStarted;
		bool m_shouldQuit;
	};
}


