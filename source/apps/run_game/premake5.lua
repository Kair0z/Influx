new_influx_app("run_game")

    -- these are the dependencies we link against
    local compile_deps = 
    {
        "influx_core",
        "influx_engine"
    }
    add_compile_dependencies(compile_deps)
    
    -- these are the dependencies not linked against,
    -- but their .dlls will be staged into this project's working dir
    local runtime_deps = 
    {
        "influx_engine",
        "influx_platform",
        "influx_input",
        "influx_async",
        "influx_import",
        "influx_renderer",
        "influx_rendergraph",
        "influx_shader",
        "influx_graphics",
        "influx_file"
    }
    add_runtime_dependencies(runtime_deps)

    staticruntime "off"