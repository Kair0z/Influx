# Influx
:wave: My personal C++ Engine/Sandbox project library!

¬ Projects Overview
----------------------
1. Apps
- [influx_editor]
- [influx_game]

2. Libraries
- [influx_import]: Library providing asset-loading functionality (PNGs, FBXs, OBJs, ...)
- [influx_async]: task scheduling
- [influx_core]: stateless, header-only library, each other library depends on this one 
- [influx_engine]: main library the applications link to
- [influx_graphics]: RHI-library (similar to Unreal Engine) providing an abstraction layer of Graphics APIs (Dx12/Vulkan)
- [influx_renderer]: the renderer used by the engine, uses influx_graphics
- ...

¬ Getting up and running
----------------------
1. Install **[GitHub Desktop for Windows](https://desktop.github.com/)** then **[fork and clone the repository](https://guides.github.com/activities/forking/)**.
- To use Git from the command line, see the [Setting up Git](https://help.github.com/articles/set-up-git/) and [Fork a Repo](https://help.github.com/articles/fork-a-repo/) articles.
- If you'd prefer not to use Git, you can get the source with the **Download ZIP** button on the right. Note that the zip utility built in to Windows marks the contents of .zip files downloaded from the Internet as unsafe to execute, so right-click the .zip file and select **Properties…** and **Unblock** before decompressing it.

2. Install **Visual Studio 2022**
To install the correct components some graphics-related code, make sure the **Game Development with C++** workload is checked. Under the **Installation Details** section on the right, also choose the following components:
-   **Windows 10 SDK** (10.0.18362 or newer)

3. Open the existing solution (.sln) file [Influx/Influx.sln]

4. Have fun compiling :)
- !!! [InfluxGame & InfluxEditor] will currently crash because
= these projects still expect us to manually copy over dependency-runtime-libraries into the output folder... (residing in 3thParty/lib/(platform)/(configuration)/...)
= there's no localized paths to shader-assets we're loading in.
- The Application-projects are grouped together and serve as runnable entrypoints into the various Library-Projects (.exe)


¬ Active Branches
------------
1. main: Should always be properly compiling!
2. graphics-project-rework: Currently working on approaching the RHI-wrapper functionality of [InfluxGraphics] in a more minimal way.
3. app-cpu-renderer: General work on [FluxRayTracing]
