
#include "InfluxApplication/Application.h"

int main()
{
	using namespace Influx::Application;

	Application::Settings appSettings{};
	appSettings.Name				= "InfluxEditor v.0.0";
	appSettings.HasWindow			= true;
	appSettings.WindowDimensions	= { 640u, 480u };
	appSettings.HasUI				= false;
	appSettings.HasSceneRender		= true;
	
	Application editorApp{ appSettings };

	editorApp.Run(0, nullptr);
}