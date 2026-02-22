-- influx app
new_influx_dll("influx_app")
    local dependencies =
    {
        "influx_core",
        "influx_platform"
    }
    set_influx_includes(dependencies)
    set_influx_links(dependencies)

    local tp_sources = 
    {
        "thirdparty/imgui"
    }
    add_thirdparty_source(tp_sources)