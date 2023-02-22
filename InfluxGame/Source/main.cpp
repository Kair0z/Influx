
#include "InfluxApplication/Application.h"

int main(int argc, char** argv)
{
	using namespace Influx::Application;

	Application::Settings appSettings{};
	appSettings.HasWindow = true;
	appSettings.WindowDimensions = { 640u, 480u };
	appSettings.HasSceneRender = true;
	appSettings.HasUI = false;
	appSettings.HasUpdate = true;
	appSettings.Name = "Flux Game 0.0";

	Application* gameApp = new Application(appSettings);

	gameApp->Run(argc, argv);

	delete gameApp;
}