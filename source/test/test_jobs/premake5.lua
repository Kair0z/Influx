new_influx_test("test_jobs")

    local dependencies =
    {
        "influx_core",
        "influx_jobs"
    }
    add_compile_dependencies(dependencies)
    add_runtime_dependencies(dependencies)
    staticruntime "off"