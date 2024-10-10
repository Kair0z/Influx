new_influx_app("influx_rendy")

    local dependencies =
    {
        "influx_core", 
        "influx_graphics", 
        "influx_input", 
        "influx_events"
    }
    set_influx_app_dependencies(dependencies)
    staticruntime "on"

    
