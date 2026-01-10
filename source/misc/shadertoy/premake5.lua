new_influx_misc("shadertoy")
    local dependencies =
    {
        "influx_core",
        "influx_platform",
        "influx_graphics",
        "influx_shader",
        "influx_rendergraph"
    }
    add_compile_dependencies(dependencies)
    add_runtime_dependencies(dependencies)
    staticruntime "off"