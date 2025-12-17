import shutil, os, argparse

parser = argparse.ArgumentParser(description='parser')

# Declare an argument (`--algo`), saying that the 
# corresponding value should be stored in the `algo` 
# field, and using a default value if the argument 
# isn't given
parser.add_argument('--config', action="store", dest='config', default='')
parser.add_argument('--game', action="store", dest='game', default='')
parser.add_argument('--deps', nargs='*')
args = parser.parse_args()

influx_root = "../"
influx_source = influx_root + "projects/influx/"
influx_source_engine = influx_source
influx_staged = influx_root + "/staged/"
influx_bin = influx_root + "/bin/" + args.config + "/"
influx_bin_game = influx_bin + args.game + "/"

print("staging binaries to " + influx_bin + " ...")

# staging game binaries
shutil.copytree(influx_bin_game, influx_staged, dirs_exist_ok=True)

# staging influx dependencies
for _, value in parser.parse_args()._get_kwargs():
    if isinstance(value, list):
        for dep in value:
            shutil.copytree(influx_bin + dep + '/', influx_staged, dirs_exist_ok=True)
            shutil.copytree(influx_bin + dep + '/', influx_bin_game, dirs_exist_ok=True)

# staging vendor dependencies
vendor_bin = influx_root + "/thirdparty/bin/"

# append the platform ('thirdparty/bin/x64/...')
if "64" in args.config:
    vendor_bin += "/x64/"
# ...

# append the config ('vendor/bin/x64/profile/...')
if "debug" in args.config:
    vendor_bin += "/debug/"
elif "profile" in args.config:
    vendor_bin += "/profile/"
elif "release" in args.config:
    vendor_bin += "/release/"

# copy into the target directories
shutil.copytree(vendor_bin, influx_staged, dirs_exist_ok=True)
shutil.copytree(vendor_bin, influx_bin_game, dirs_exist_ok=True)
    



