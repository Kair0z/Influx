import shutil, os

print("staging content...")
influx_root = "../"
influx_resources = influx_root + "/resources/"
influx_staged = influx_root + "/staged/"
influx_staged_resources = influx_staged + "/resources/"
shutil.copytree(influx_resources, influx_staged_resources, dirs_exist_ok=True)