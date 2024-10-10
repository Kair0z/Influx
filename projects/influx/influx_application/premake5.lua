-- influx application
new_influx_dll("influx_application")

    pchheader "app_pch.h"
    pchsource ("source/app_pch.cpp")

    local dependencies =
    {
        "influx_core",
        "influx_renderer",
        "influx_async",
        "influx_import",
        "influx_input",
        "influx_events",
        "influx_shader"
    }

    set_influx_includes(dependencies)
    set_influx_links(dependencies)

    -- remove imgui pch
    filter "files:**/imgui/**.cpp"
        flags {"NoPCH"}

        
