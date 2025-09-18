project "cpu_raytracer"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    staticruntime "on"

    g_project_dir = g_dir_source_misc .. "/%{prj.name}/"

    targetdir(g_dir_binaries .. "/%{prj.name}")
    objdir(g_dir_int .. "/%{prj.name}")

    files
    {
        g_project_dir .. "/**.cpp",
        g_project_dir .. "/**.h",
        g_project_dir .. "/**.lua"
    }

    defines
    {
        
    }

    includedirs
    {
        "include",
        "source",
        g_dir_core_include,
        g_dir_vendor_include
    }

    links
    {

    }
