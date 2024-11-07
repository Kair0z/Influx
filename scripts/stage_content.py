import shutil, os

influx_root = "../"
influx_assets = influx_root + "/assets/"
influx_staged = influx_root + "/staged/"
influx_staged_assets = influx_staged + "/assets/"

print("staging assets...")
shutil.copytree(influx_assets, influx_staged_assets, dirs_exist_ok=True)