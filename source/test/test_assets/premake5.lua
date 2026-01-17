new_influx_test("test_assets")

    local runtime_deps = 
    {
        "influx_assets",
        "influx_shader",
        "influx_import",

        "thirdparty/assimp",
        "thirdparty/slang"
    }
    add_runtime_dependencies(runtime_deps)

    local dependencies =
    {
        "influx_core",
        "influx_assets"
    }
    add_compile_dependencies(dependencies)
    staticruntime "off"