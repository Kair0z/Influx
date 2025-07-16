new_influx_test("test_profiling")

    local dependencies =
    {
        "influx_core"
    }
    set_influx_app_dependencies(dependencies)
    staticruntime "off"