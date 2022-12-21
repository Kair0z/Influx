
#include "Application.h"

int main()
{
	using namespace Influx::Application;

	Application::Settings appSettings{};
	appSettings.Name				= "InfluxEditor v.0.0";
	appSettings.HasWindow			= true;
	appSettings.WindowDimensions	= { 1600, 900u };
	appSettings.HasUI				= true;

	Application* editorApp = new Application(appSettings);
	editorApp->Run(0, nullptr);
}