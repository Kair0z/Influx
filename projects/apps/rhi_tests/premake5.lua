new_influx_app("rhi_tests")

    local dependencies =
    {
        "influx_core",
        "influx_platform",
        "influx_file",
    }
    set_influx_app_dependencies(dependencies)
    staticruntime "off"