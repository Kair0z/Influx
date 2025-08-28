new_influx_misc("rendering")
    local dependencies =
    {
        "influx_core",
        "influx_platform",
        "influx_graphics",
        "influx_rendergraph",
        "influx_imgui",
        "influx_shader",
        "influx_import"
    }
    set_influx_app_dependencies(dependencies)
    staticruntime "off"