
#include "InfluxApplication/Application.h"

/*
*	[INFLUX EDITOR]
* 
* 
*/

int main(int argc, char** argv)
{
	using namespace Influx::Application;

	Application::Settings appSettings{};
	appSettings.Name = "InfluxEditor v.0.0";
	appSettings.HasWindow = true;
	appSettings.WindowDimensions = { 640u, 480u };
	appSettings.HasImGUI = false;
	appSettings.HasSceneRender = true;

	{
		Application editorApp{ appSettings };

		editorApp.Run(argc, argv);
	}
}