project "shaders"
    kind "None"

    files
    {
        g_dir_assets .. "shaders/**.hlsl",
        g_dir_assets .. "shaders/**.hlsli"
    }
