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
        "influx_graphics",
        "influx_imgui",
        "influx_file"
    }

    set_influx_includes(dependencies)
    set_influx_links(dependencies)

    local tp_sources = 
    {
        "thirdparty/imgui",
        "thirdparty/json",
        "thirdparty/tom++",
        "thirdparty/entt"
    }
    add_thirdparty_source(tp_sources)

    -- remove imgui pch
    filter "files:**/imgui/**.cpp"
        flags {"NoPCH"}