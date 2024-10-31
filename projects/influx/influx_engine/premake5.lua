-- influx engine
new_influx_dll("influx_engine")

    pchheader "engine_pch.h"
    pchsource ("source/engine_pch.cpp")

    local dependencies =
    {
        "influx_core",
        "influx_platform",
        "influx_input",
        "influx_async",
        "influx_import",
        "influx_shader",
        "influx_renderer",
        "influx_imgui",
    }

    set_influx_includes(dependencies)
    set_influx_links(dependencies)

    -- remove imgui pch
    filter "files:**/imgui/**.cpp"
        flags {"NoPCH"}

        
