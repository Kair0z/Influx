import shutil, os

influx_root = "../"
influx_projects = influx_root + "projects/influx/"
influx_projects_engine = influx_projects

# for each project in ../projects/influx/
for project_name in os.listdir(influx_projects_engine):
    # for 
    include_src_dir = influx_projects_engine + project_name + "/include/"
    include_tar_dir = "../include/" + project_name + "/"
    print(include_src_dir + " -> " + include_tar_dir)
    shutil.copytree(include_src_dir, include_tar_dir, dirs_exist_ok=True)

print('include files copied :)')