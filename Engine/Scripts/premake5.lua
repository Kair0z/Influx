-- From Influx/Engine/Scripts/.. Towards Influx/

workspace "Influx"
    location("../..")
    architecture "x64"
    configurations {"Debug", "Release", "DebugEditor", "ReleaseEditor"}

SLN_BASE_DIR = "%{wks.location}"
ENGINE_BASE_DIR = SLN_BASE_DIR.. "/Engine/"

-- windows/x64/Debug/...
OUT_SYS_PLATFORM_CONFIG = "%{cfg.system}/x64/%{cfg.buildcfg}"

-- DLL output
DLL_OUTPUT = SLN_BASE_DIR .. "/Engine/Bin/" .. OUT_SYS_PLATFORM_CONFIG

-- [INFLUX ENGINE] --
print("GENERATING INFLUX ENGINE")

    -- Influx_XX
    projname = "Influx"

    src_dir = ENGINE_BASE_DIR .. "/Source/"

    -- Engine/Bin/{}/Influx_XX
    target_dir = ENGINE_BASE_DIR .. "Bin/" ..OUT_SYS_PLATFORM_CONFIG
    int_dir = ENGINE_BASE_DIR .. "Int/" ..OUT_SYS_PLATFORM_CONFIG

    project(projname)
        location(SLN_BASE_DIR .. "/Engine/")
        kind "StaticLib"
        language "C++"

        targetdir(target_dir)
        objdir(int_dir)

        print("Using Files @ " .. src_dir.. "/src/...")
        pchheader "pch.h"
        pchsource(src_dir.."pch.cpp")
        files
        {
            src_dir .. "/**.h",
            src_dir .. "/**.cpp",
            src_dir .. "/**.inl"
        }

        includedirs
        {
            src_dir,
            ENGINE_BASE_DIR .. "/3thParty/include/"
        }

        links
        {
            
        }

        postbuildcommands
        {
            --"{COPY} %{cfg.targetdir}/**.dll " ..dlloutdir,
            --"{COPY} %{cfg.targetdir}/**.dll " .. DLL_OUTPUT 
        }

        filter "system:windows"
            cppdialect "C++17"

        linkoptions { "-IGNORE:4006" }
        a3thParty_Base_Lib_Dir = ENGINE_BASE_DIR .. "/3thParty/lib/"
        
        filter "configurations:Debug"
            defines {"DEBUG", "PLATFORM_WINDOWS"}
            symbols "On"
            libdirs {a3thParty_Base_Lib_Dir .. "/Debug/"}

        filter "configurations:DebugEditor"
            defines {"DEBUG", "WITH_EDITOR", "PLATFORM_WINDOWS"}
            symbols "On"
            libdirs {a3thParty_Base_Lib_Dir .. "/Debug/"}

        filter "configurations:Release"
            defines {"RELEASE", "PLATFORM_WINDOWS"}
            optimize "On"
            libdirs {a3thParty_Base_Lib_Dir .. "/Release/"}

        filter "configurations:ReleaseEditor"
            defines {"RELEASE", "WITH_EDITOR", "PLATFORM_WINDOWS"}
            optimize "On"
            libdirs {a3thParty_Base_Lib_Dir .. "/Release/"}


-- [SANDBOX] --
print("GENERATING SANDBOX")

    -- Influx_XX
    projname = "Sandbox"

    src_dir = SLN_BASE_DIR .. "/Sandbox/Source/"

    -- Engine/Bin/{}/Influx_XX
    target_dir = SLN_BASE_DIR .. "/Sandbox/Bin/" ..OUT_SYS_PLATFORM_CONFIG.. "/%{prj.name}"
    int_dir = SLN_BASE_DIR .. "/Sandbox/Int/" ..OUT_SYS_PLATFORM_CONFIG.. "/%{prj.name}"

    project(projname)
        location(SLN_BASE_DIR .. "/" .. projname)
        kind "ConsoleApp"
        language "C++"

        targetdir(target_dir)
        objdir(int_dir)

        print("Using Files @ " .. src_dir.. "/src/...")
        files{
            src_dir .. "/**.h",
            src_dir .. "/**.cpp",
            src_dir .. "/**.inl"
        }

        includedirs
        {
            ENGINE_BASE_DIR .. "/Source/"
        }

        links
        {
            ENGINE_BASE_DIR .. "Bin/" ..OUT_SYS_PLATFORM_CONFIG.. "/Influx"
        }

        postbuildcommands
        {
            --"{COPY} %{cfg.targetdir}/**.dll " ..dlloutdir,
            --"{COPY} %{cfg.targetdir}/**.dll " .. DLL_OUTPUT 
        }

        defines
        {
            
        }

        filter "system:windows"
            cppdialect "C++17"

        filter "configurations:Debug"
            defines {"DEBUG", "PLATFORM_WINDOWS"}
            symbols "On"
            --libdirs {EXT_PARTY_BASE_LIB_DIR .. "/Debug/"}

        filter "configurations:DebugEditor"
            defines {"DEBUG", "WITH_EDITOR", "PLATFORM_WINDOWS"}
            symbols "On"
            --libdirs {EXT_PARTY_BASE_LIB_DIR .. "/Debug/"}

        filter "configurations:Release"
            defines {"RELEASE", "PLATFORM_WINDOWS"}
            optimize "On"
            --libdirs {EXT_PARTY_BASE_LIB_DIR .. "/Release/"}

        filter "configurations:ReleaseEditor"
            defines {"RELEASE", "WITH_EDITOR", "PLATFORM_WINDOWS"}
            optimize "On"
            --libdirs {EXT_PARTY_BASE_LIB_DIR .. "/Release/"}
            
        