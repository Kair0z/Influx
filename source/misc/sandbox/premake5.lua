new_influx_misc("sandbox")
    local dependencies =
    {
        "influx_core",
        "influx_platform",
        "influx_rhi",
        "influx_shader"
    }
    add_compile_dependencies(dependencies)
    add_runtime_dependencies(dependencies)
    staticruntime "off"