new_influx_test("test_async")

    local dependencies =
    {
        "influx_core",
        "influx_async"
    }
    set_influx_app_dependencies(dependencies)
    staticruntime "off"

    g_use_tracy = false
    if g_use_tracy then
        files
        {
            g_dir_tracy_source .. "**.h",
            g_dir_tracy_source .. "**.cpp"
        }
    end