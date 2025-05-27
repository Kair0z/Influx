new_influx_test("test_async")

    local dependencies =
    {
        "influx_core",
        "influx_async"
    }
    set_influx_app_dependencies(dependencies)
    staticruntime "off"