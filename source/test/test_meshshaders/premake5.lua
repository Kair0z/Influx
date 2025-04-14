new_influx_test("test_meshshaders")

    local dependencies =
    {
        "influx_core",
        "influx_platform",
        "influx_graphics",
        "influx_shader"
    }
    set_influx_app_dependencies(dependencies)
    staticruntime "off"