#include "core/result.h"
#include <iostream>

#include "core/time.h"

using namespace influx;
result<> res_do_something()
{
	new int();

	if (true)
	{
		return {};
	}
	else
	{
		return true;
	}
}

bool do_something()
{
	new int();
	return true;
}

int main()
{
}