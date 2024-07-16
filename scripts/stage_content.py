import shutil, os

influx_root = "../"
influx_resources = influx_root + "/resources/"
influx_assets = influx_root + "/assets/"
influx_staged = influx_root + "/staged/"
influx_staged_resources = influx_staged + "/resources/"
influx_staged_assets = influx_staged + "/assets/"

print("staging resources...")
shutil.copytree(influx_resources, influx_staged_resources, dirs_exist_ok=True)

print("staging assets...")
shutil.copytree(influx_assets, influx_staged_assets, dirs_exist_ok=True)