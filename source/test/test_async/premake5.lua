new_influx_test("test_async")

    local dependencies =
    {
        "influx_core",
        "influx_async"
    }
    add_compile_dependencies(dependencies)
    add_runtime_dependencies(dependencies)
    staticruntime "off"

    g_use_tracy = false
    if g_use_tracy then
        files
        {
            g_dir_tracy_source .. "**.h",
            g_dir_tracy_source .. "**.cpp"
        }
    end