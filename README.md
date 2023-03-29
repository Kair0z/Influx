# Influx
:wave: My personal C++ Engine/Sandbox project library!

¬ Projects Overview
----------------------
1. Apps
- [FluxRenderer]: Minimalist Renderer showcasing the [InfluxGraphics] project-library
- [InfluxEditor]: Editor application hosting the [InfluxEngine] project-library
- [InfluxGame]: Game application hosting the [InfluxEngine] project-library
- [InfluxRayTracing]: Standalone CPU-Raytracer application using [InfluxCore]

2. Libraries
- [InfluxCore]: Header-only library providing various basic-types, math-& utility-functions 
- [InfluxApplication]: Library providing a Windowed Application interface that hosts [InfluxEngine]
- [InfluxEngine]: ... Honestly not quite sure yet what functionality will reside in this centralized unit...
- [InfluxGraphics]: RHI-library (similar to Unreal Engine) providing an abstraction layer of Graphics APIs (Dx12/Vulkan)
- [InfluxRenderer]: Library providing a Renderer interface using [InfluxGraphics]
- [InfluxAssets]: Library providing asset-loading functionality (PNGs, FBXs, OBJs, ...)
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

4. Build 1 or any other of the Application.

¬ Active Branches
------------
1. main: Should always be properly compiling!
2. graphics-project-rework: Currently working on approaching the RHI-wrapper functionality of [InfluxGraphics] in a more minimal way.
3. app-cpu-renderer: General work on [FluxRayTracing]
