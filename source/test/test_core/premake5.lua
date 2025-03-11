new_influx_test("test_core")

    local dependencies =
    {
        "influx_core"
    }
    set_influx_app_dependencies(dependencies)
    staticruntime "off"