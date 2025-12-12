new_influx_test("test_jobs")

    local dependencies =
    {
        "influx_core",
        "influx_jobs"
    }
    set_influx_app_dependencies(dependencies)
    staticruntime "off"