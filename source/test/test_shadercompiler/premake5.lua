new_influx_test("test_shadercompiler")

    local dependencies =
    {
        "influx_core",
        "influx_shader",
        "influx_import"
    }
    set_influx_app_dependencies(dependencies)
    staticruntime "off"