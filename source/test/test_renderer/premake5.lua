new_influx_test("test_renderer")

    local compile_deps =
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
    add_compile_dependencies(compile_deps)

    local runtime_deps =
    {
        "influx_platform",
        "influx_graphics",
        "influx_shader",
        "influx_renderer",
        "influx_rendergraph",
        "influx_import",
        "influx_async",

        "thirdparty/assimp",
        "thirdparty/slang",
        "thirdparty/DXC",
        "thirdparty/D3D12"
    }
    add_runtime_dependencies(runtime_deps)
    staticruntime "off"

