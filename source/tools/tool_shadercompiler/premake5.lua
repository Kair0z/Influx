new_influx_tool("tool_shadercompiler")
    local dependencies =
    {
        "influx_core",
        "influx_platform",
        "influx_shader"
    }
    add_compile_dependencies(dependencies)

    local runtime_deps = 
    {
        "influx_core",
        "influx_platform",
        "influx_shader",

        "thirdparty/DXC/"
    }
    add_runtime_dependencies(runtime_deps)
    staticruntime "off"