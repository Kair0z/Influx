new_influx_misc("virtualbox")
    local dependencies =
    {
        "influx_core",
        "influx_platform",
        "influx_rhi",
        "influx_shader",
        "influx_import"
    }
    add_compile_dependencies(dependencies)
    add_runtime_dependencies(dependencies)
    staticruntime "off"