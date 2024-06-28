import shutil, os

influx_root = "../"
influx_projects = influx_root + "projects/influx/"
influx_projects_engine = influx_projects

#
for project_name in os.listdir(influx_projects_engine):
    print(project_name)
    for root, dirs, files in os.walk(influx_projects_engine + project_name):
        for file in files:
            full_file = os.path.join(root, file)
            if full_file.__contains__('include'):
                full_file.replace(influx_projects_engine + project_name + '/include/', '')
                print(full_file)
                shutil.copy(full_file, '../include/')


print('done')