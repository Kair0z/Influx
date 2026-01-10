new_influx_test("test_core")

    local dependencies =
    {
        "influx_core"
    }
    add_compile_dependencies(dependencies)
    add_runtime_dependencies(dependencies)
    staticruntime "off"