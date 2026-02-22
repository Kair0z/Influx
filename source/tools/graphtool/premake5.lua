new_influx_tool("graphtool")
    local dependencies =
    {
        "influx_core",
        "influx_platform",
        "influx_app"
    }
    add_compile_dependencies(dependencies)
    staticruntime "off"

    local tp_sources = 
    {
        "thirdparty/imgui"
    }
    add_thirdparty_source(tp_sources)