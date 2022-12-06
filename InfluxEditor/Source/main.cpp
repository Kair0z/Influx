
#include "Application.h"

int main()
{
	using namespace Influx;
	ApplicationDescription appDesc{};
	appDesc.Type = EApplicationType::Default;
	appDesc.InitWindowSize = { 640u, 480u };
	appDesc.Name = "InfluxEditor v.0.0";

	Application* editorApp = new Application(0, nullptr, appDesc);
	editorApp->Run([]()
		{
			ImGui::Begin("Hello");

			ImGui::Button("Button");

			ImGui::End();

			ImGui::ShowDemoWindow();
		});
}