-- influx jobs
new_influx_library("influx_jobs")
    pchheader "jobs_pch.h"
    pchsource ("source/jobs_pch.cpp")

    local dependencies =
    {
        "influx_core"
    }
    set_influx_includes(dependencies)
    set_influx_links(dependencies)