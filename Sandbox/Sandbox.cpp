
#include "InfluxAssets/InfluxAssets.h"

int main()
{
	using namespace Influx;

	Assets::Scene scene{};
	Assets::SceneCache sceneCache{};

	bool loadSuccesful = Assets::LoadScene("Path", scene, &sceneCache);
}