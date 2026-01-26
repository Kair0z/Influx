new_influx_test("test_pipelines")

    local dependencies =
    {
        "influx_core",
        "influx_platform",
        "influx_renderer",
        "influx_graphics",
        "influx_shader"
    }
    add_compile_dependencies(dependencies)

    local runtime_deps =
    {
        "influx_platform",
        "influx_renderer",
        "influx_graphics",
        "influx_shader",
        "influx_rendergraph",

        "thirdparty/DXC",
        "thirdparty/D3D12"
    }
    add_runtime_dependencies(runtime_deps)
    staticruntime "off"