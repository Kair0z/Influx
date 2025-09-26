new_influx_misc("virtualbox")
    local dependencies =
    {
        "influx_core",
        "influx_platform",
        "influx_rhi",
        "influx_shader",
        "influx_import"
    }
    set_influx_app_dependencies(dependencies)
    staticruntime "off"