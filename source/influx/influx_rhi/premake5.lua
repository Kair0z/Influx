-- influx graphics
new_influx_library("influx_rhi")

    pchheader "rhi_pch.h"
    pchsource "source/rhi_pch.cpp"

    local dependencies =
    {
        "influx_core",
        "influx_platform"
    }

    set_influx_includes(dependencies)
    set_influx_links(dependencies)

    g_compile_vulkan = true
    g_compile_d3d12 = false

    removefiles
    {
        iif(g_compile_vulkan ~= true, "**/vulkan/**", ""),
        iif(g_compile_d3d12 ~= true, "**/d3d12/**", "")
    }

    defines
    {
        iif(g_compile_vulkan, "INFLUX_VULKAN=1", "INFLUX_VULKAN=0"),
        iif(g_compile_d3d12, "INFLUX_DX12=1", "INFLUX_DX12=0")
    }

    includedirs
    {
        "foreign/",
        iif(g_compile_vulkan, "foreign/vulkan/", ""),
        iif(g_compile_d3d12, "foreign/d3d12/", ""),
    }

    links
    {
        -- iif(g_compile_d3d12, "d3d12", ""),
        -- iif(g_compile_d3d12, "dxgi", ""),
        iif(g_compile_vulkan, "vulkan-1", "")
    }
