new_influx_test("test_app")

    local dependencies =
    {
        "influx_core",
        "influx_platform",
        "influx_app"
    }
    add_compile_dependencies(dependencies)
    add_runtime_dependencies(dependencies)
    staticruntime "off"