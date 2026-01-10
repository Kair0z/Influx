new_influx_test("test_renderer")

    local dependencies =
    {
        "influx_core",
        "influx_platform",
        "influx_graphics",
        "influx_shader",
        "influx_renderer",
        "influx_rendergraph",
        "influx_import",
        "influx_async"
    }
    add_compile_dependencies(dependencies)
    add_runtime_dependencies(dependencies)
    staticruntime "off"

