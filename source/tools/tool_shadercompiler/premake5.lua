new_influx_tool("tool_shadercompiler")
    local dependencies =
    {
        "influx_core",
        "influx_platform",
        "influx_shader"
    }
    add_compile_dependencies(dependencies)
    add_runtime_dependencies(dependencies)
    staticruntime "off"