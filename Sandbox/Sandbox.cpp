
#include "InfluxAssets/InfluxAssets.h"

#pragma comment(lib, "InfluxAssets.lib")

#include <iostream>

int main()
{
	using namespace Influx::Assets;

	Mesh mesh{};
	bool success = LoadMesh("E:/Git/Influx/Resources/Meshes/CafeLeBlanc.fbx", mesh);

	std::cin.get();
}