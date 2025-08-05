import shutil, os

influx_root = "../"
influx_source = influx_root + "source/influx/"
influx_source_engine = influx_source

# for each project in ../source/influx/
for project_name in os.listdir(influx_source_engine):
    # for 
    include_src_dir = influx_source_engine + project_name + "/include/"
    include_tar_dir = "../include/" + project_name + "/"
    print(include_src_dir + " -> " + include_tar_dir)
    shutil.copytree(include_src_dir, include_tar_dir, dirs_exist_ok=True)

print('include files copied :)')