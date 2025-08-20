# Influx
:wave: This repository stored my personal C++ Engine/Sandbox project library!

¬ C++ Project Info
-------------------
C++ version: C++20

¬ Projects Overview
----------------------
1. apps
- [pong]: game dll that gets loaded at runtime by either the editor or game application
- [run_game]: application that runs influx_engine in 'game-mode'
- [run_editor]: application that runs influx_engine in 'editor-mode'

2. Libraries
- [influx_async]: wraps a multithreaded task-manager
- [influx_core]: stateless header-only library, used by pretty much each influx library 
- [influx_engine]: main static library the runnable applications link to
- [influx_graphics]: wraps an abstraction layer of Graphics APIs (Dx12/Vulkan) (similar to Unreal Engine RHI)
- [influx_rhi]: ALSO wraps an abstraction layer. this is an experimental template-heavy version of influx_graphics
- [influx_imgui]: wraps imgui to use our RHI abstraction layer
- [influx_import]: wraps third-party asset-loading (PNGs, FBXs, OBJs, ...)
- [influx_input]: wraps input handling
- [influx_platform] wraps platform API (Win32 only for now)
- [influx_renderer] wraps the influx _standalone_ renderer.
- [influx_rendergraph] wraps a rendergraph implementation
- [influx_shader] wraps shader compilation logic
- ...
  
3. misc
- projects that are minimal standalone (not heavy reliant on the influx_engine structure)
5. test
- tests for the corresponding influx_library projects.
6. thirdparty
7. tools

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

¬ Active Branches
------------
1. main: Should always be properly compiling!
