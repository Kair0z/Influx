new_influx_misc("sandbox")
    local dependencies =
    {
        "influx_core",
        "influx_platform",
        "influx_rhi",
        "influx_shader"
    }
    set_influx_app_dependencies(dependencies)
    staticruntime "off"