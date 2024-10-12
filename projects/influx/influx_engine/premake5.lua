-- influx engine
new_influx_statlib("influx_engine")

    pchheader "engine_pch.h"
    pchsource ("source/engine_pch.cpp")

    local dependencies =
    {
        "influx_core",
        "influx_platform"
    }

    set_influx_includes(dependencies)
    set_influx_links(dependencies)

    -- remove imgui pch
    filter "files:**/imgui/**.cpp"
        flags {"NoPCH"}

        
