#include <iostream>

#include "Core\Platform\Platform.h"

struct Data
{
	float x;
};
int main()
{
	Data* pData = Influx::Platform::Allocate<Data>();
	Influx::Platform::Free(pData);
}