
#include "core/file.h"
#include <iostream>

int main()
{
	using namespace influx;

	file_watcher watcher{ file("E:/Data/file.txt") };

	watcher.subscribe_onchange([](const file& file)
	{
		std::cout << "file changed! \n";
	});

	watcher.subscribe_onrename([](const file& file)
	{
		std::cout << "file renamed! \n";
	});

	while (true)
	{
		watcher.check_file();
	}
}