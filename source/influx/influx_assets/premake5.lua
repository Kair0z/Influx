new_influx_library("influx_assets")

	local tp_sources = 
    {
        "thirdparty/tom++"
    }
    add_thirdparty_source(tp_sources)

	local dependencies =
	{
		"influx_core",
		"influx_import",
		"influx_shader"
	}
	add_compile_dependencies(dependencies)