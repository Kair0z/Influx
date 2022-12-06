
#include "Application.h"

void OnApplicationUpdate(Influx::Application& app)
{

}

void OnApplicationUIRender(const Influx::Application& app)
{

}

int main()
{
	using namespace Influx;
	ApplicationDescription appDesc{};
	appDesc.Type = EApplicationType::Default;
	appDesc.InitWindowSize = { 640u, 480u };
	appDesc.Name = "InfluxEditor v.0.0";

	Application* editorApp = new Application(0, nullptr, appDesc);

	editorApp->SetUIRenderCallback(OnApplicationUIRender);
	editorApp->SetUpdateCallback(OnApplicationUpdate);

	editorApp->Run();
}