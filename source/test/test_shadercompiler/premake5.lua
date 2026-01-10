new_influx_test("test_shadercompiler")

    local dependencies =
    {
        "influx_core",
        "influx_shader",
        "influx_import"
    }
    add_compile_dependencies(dependencies)
    add_runtime_dependencies(dependencies)
    staticruntime "off"