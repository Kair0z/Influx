new_influx_test("test_app")

    local dependencies =
    {
        "influx_core",
        "influx_platform",
        "influx_app"
    }
    set_influx_app_dependencies(dependencies)
    staticruntime "off"