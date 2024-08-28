project "shaders"
    kind "None"

    files
    {
        g_dir_resources .. "shaders/**.hlsl",
        g_dir_resources .. "shaders/**.hlsli"
    }
