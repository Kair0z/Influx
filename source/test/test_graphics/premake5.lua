new_influx_test("test_graphics")

    local dependencies =
    {
        "influx_core",
        "influx_platform",
        "influx_graphics"
    }
    set_influx_app_dependencies(dependencies)
    staticruntime "off"