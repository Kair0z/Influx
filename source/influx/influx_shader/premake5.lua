new_influx_library("influx_shader")

    shader_backend_slang = true 
    local dependencies =
    {
        "influx_core",
        "thirdparty/slang",

        "thirdparty/dxc"
    }
    add_compile_dependencies(dependencies)