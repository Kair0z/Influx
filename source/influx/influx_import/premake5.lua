-- influx assets
new_influx_library("influx_import")

    pchheader "import_pch.h"
    pchsource ("source/import_pch.cpp")

    local dependencies =
    {
        "influx_core",
        "influx_shader",

        "thirdparty/assimp"
    }

    set_influx_includes(dependencies)
    set_influx_links(dependencies)

    disablewarnings 
    {
        "4244",
        "4267"
    }

    filter "files:**/lodepng/**.cpp"
        flags {"NoPCH"}