#include <iostream>

#include "Core/Platform/WindowsPlatform.h"

struct Data
{
	float x;
};
int main()
{
	Data* pData = Influx::Platform::Allocate<Data>();
	Influx::Platform::Free(pData);
}