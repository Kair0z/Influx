new_influx_library("influx_shader")

    shader_backend_slang = true 
    local dependencies =
    {
        "influx_core",
        "thirdparty/slang"
    }
    set_influx_includes(dependencies)
    set_influx_links(dependencies)