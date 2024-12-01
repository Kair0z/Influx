
#include "core/filewatcher.h"
#include <iostream>

int main()
{
	using namespace influx;

	file_watcher watcher{ file("E:/Data/file.txt") };

	auto on_file_change = [](const file& file)
	{
		std::cout << "file renamed! \n";
	};
	watcher.subscribe<file_watcher::on_change>(on_file_change);

	while (true)
	{
		watcher.check_file();
	}

	watcher.unsub<file_watcher::on_change>(on_file_change);
}