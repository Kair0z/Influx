
#include "Application.h"

int main()
{
	using namespace Influx::Application;
	Application::Settings appSettings{};
	appSettings.Type = EApplicationType::ImGuiApp;
	appSettings.WindowDimensions = { 640u, 480u };
	appSettings.Name = "InfluxEditor v.0.0";

	Application* editorApp = new Application(0, nullptr, appSettings);
	editorApp->Run();
}