
#include "influx_script.h"

class my_script : influx::script::script
{
public:
	virtual void on_start() override
	{

	}

	virtual void on_update()
	{

	}

	virtual const char* print()
	{
		return "my_script";
	}
};