new_influx_tool("tool_shadercompiler")
    local dependencies =
    {
        "influx_core",
        "influx_platform",
        "influx_shader"
    }
    set_influx_app_dependencies(dependencies)
    staticruntime "off"