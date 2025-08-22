new_influx_test("test_rendergraph")

    local dependencies =
    {
        "influx_core",
        "influx_platform",
        "influx_graphics",
        "influx_rhi",
        "influx_rendergraph",
        "influx_shader"
    }
    set_influx_app_dependencies(dependencies)
    staticruntime "off"