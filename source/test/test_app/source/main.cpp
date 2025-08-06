
#include "influx_app.h"
#include <iostream>

int main()
{
    using namespace influx;
    
    app app{app::e_component_flags::console | app::e_component_flags::window};

    // initial settings
    app::window_settings winsettings{};
    winsettings.m_title = "influx app";
    winsettings.m_dimensions.x = 640u;
    winsettings.m_dimensions.y = 480u;
    app.set_settings<app::e_settings::window>(winsettings);
    app::console_settings consettings{};
    app.set_settings<app::e_settings::console>(consettings);

    // run the app on a thread
    app.run(app::e_runmode::run_on_thread);

    // on click, quit the app and end the process
    while (app.is_running())
    {
        app.quit();
    }
}