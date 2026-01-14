new_influx_test("test_core")

    local dependencies =
    {
        "influx_core"
    }
    add_compile_dependencies(dependencies)
    add_runtime_dependencies(dependencies)

    local tp_sources = 
    {
        "thirdparty/glm"
    }
    -- add_thirdparty_source(tp_sources)
    staticruntime "off"