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
    add_compile_dependencies(dependencies)

    local tp_sources = 
    {
        "thirdparty/lodepng",
        "thirdparty/cereal"
    }
    add_thirdparty_source(tp_sources)

    disablewarnings 
    {
        "4244",
        "4267"
    }

    filter "files:**/lodepng/**.cpp"
        flags {"NoPCH"}