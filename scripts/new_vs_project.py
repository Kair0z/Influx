import sys, os
REQUIRED_PYTHON = (3, 8)
if sys.version_info < REQUIRED_PYTHON:
    sys.exit(f"Python {REQUIRED_PYTHON[0]}.{REQUIRED_PYTHON[1]} or above is required. You are using Python {sys.version_info.major}.{sys.version_info.minor}.")

influx_root             = "../"
influx_source           = influx_root + "/source/"
influx_source_apps      = influx_source + "/apps/"
influx_source_engine    = influx_source + "/influx/"
influx_source_misc      = influx_source + "/misc/"

if len(sys.argv) > 1:  # Ensure there are arguments
    arg1 = sys.argv[1]  # First argument: project name

    project_name = arg1
    project_root = influx_source_engine + "/" + project_name + "/"

    print("creating " + project_name + " ..")

    # create the source/include folders
    os.makedirs(project_root + "/include/", exist_ok=True)
    os.makedirs(project_root + "/source", exist_ok=True)

    # write the premake file
    with open(project_root + "premake5.lua", "w") as file:
        file.write("new_influx_library(\"" + project_name + "\")\n")
        file.write("\n")
        file.write("\tlocal dependencies =\n")
        file.write("\t{\n")
        file.write("\t\t\"influx_core\"\n")
        file.write("\t}\n")
        file.write("\tset_influx_includes(dependencies)\n")
        file.write("\tset_influx_links(dependencies)\n")

    # open the folder
    os.startfile(os.path.abspath(project_root))