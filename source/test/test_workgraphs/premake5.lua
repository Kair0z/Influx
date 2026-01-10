new_influx_test("test_workgraphs")
    local dependencies =
    {
        "influx_core",
        "influx_platform",
        "influx_graphics",
        "influx_shader"
    }
    add_compile_dependencies(dependencies)
    add_runtime_dependencies(dependencies)
    staticruntime "off"

    files
    {
        project_dir .. "**.hlsl"
    }