import shutil, os, argparse

# parse the arguments
parser = argparse.ArgumentParser(description='parser')
parser.add_argument('--config', action="store", dest='config', default='')
parser.add_argument('--game', action="store", dest='game', default='')
parser.add_argument('--deps', nargs='*')
args = parser.parse_args()

# DONT CHANGE THIS WITHOUT CHANGING g_thirdparty_prefix in premake.lua!!!
thirdparty_prefix = "thirdparty/"

influx_root = "../"
influx_source = influx_root + "projects/influx/"
influx_source_engine = influx_source
influx_bin = influx_root + "/bin/" + args.config + "/"
thirdparty_bin = influx_root + "/thirdparty/bin/"

# append to thirdparty/bin/ the config & platform
if "64" in args.config:
    thirdparty_bin += "/x64/"
if "debug" in args.config:
    thirdparty_bin += "/debug/"
elif "profile" in args.config:
    thirdparty_bin += "/profile/"
elif "release" in args.config:
    thirdparty_bin += "/release/"

# this is the target folder we're staging into:
target_dir = influx_bin + args.game + "/"

print("staging binaries to " + influx_bin + " ...")

# staging influx dependencies
def ignore_non_dll(dir, entries):
    ignored = []
    for entry in entries:
        full_path = os.path.join(dir, entry)
        if os.path.isfile(full_path) and not entry.lower().endswith(".dll"):
            ignored.append(entry)
    return ignored

for _, value in parser.parse_args()._get_kwargs():
    if isinstance(value, list):
        for dep in value:
            if thirdparty_prefix in dep:
                # thirdparty/dep case:
                name_without_prefix = dep.replace(thirdparty_prefix, "")
                thirdparty_folder = thirdparty_bin + name_without_prefix + "/"
                #print(thirdparty_folder)
                shutil.copytree(thirdparty_folder, target_dir, dirs_exist_ok=True, ignore=ignore_non_dll)
            else:
                # influx_dep case:
                shutil.copytree(influx_bin + dep + '/', target_dir, dirs_exist_ok=True, ignore=ignore_non_dll)



