
#include "Influx.h"

int main()
{
    Influx::Engine* pEngine = new Influx::Engine();
    pEngine->Run();

    delete pEngine;
}

