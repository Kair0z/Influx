#include "influx_game.h"
#include <iostream>

void game::start();
{
	printf("game:on_start\n");

}

void game::tick()
{
	printf("game:on_tick\n");


}

void game::end()
{
	printf("game:on_end\n");
}