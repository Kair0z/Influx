new_influx_runnable("toolbox")
    local dependencies =
    {
        "influx_core",
        "influx_platform",
        "influx_app"
    }
    add_compile_dependencies(dependencies)

    local runtime_deps =
    {
        "influx_core",
        "influx_platform",
        "influx_app",

        "thirdparty/DXC/"
    }
    add_runtime_dependencies(runtime_deps)
    
    staticruntime "off"