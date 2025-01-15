-- influx shadertoy
new_influx_library("influx_shadertoy")
    local dependencies =
    {
        "influx_core",
        "influx_platform",
        "influx_graphics",
        "influx_shader"
    }
    set_influx_includes(dependencies)
    set_influx_links(dependencies)
