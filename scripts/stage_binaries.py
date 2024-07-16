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

print("staging binaries...")
influx_root = "../"
influx_projects = influx_root + "projects/influx/"
influx_projects_engine = influx_projects
influx_staged = influx_root + "/staged/"
influx_bin = influx_root + "/bin/" + args.config + "/"
influx_bin_game = influx_bin + args.game + "/"

# staging game binaries
shutil.copytree(influx_bin_game, influx_staged, dirs_exist_ok=True)

# staging game dependencies
for _, value in parser.parse_args()._get_kwargs():
    if isinstance(value, list):
        for dep in value:
            shutil.copytree(influx_bin + dep + '/', influx_staged, dirs_exist_ok=True)
            shutil.copytree(influx_bin + dep + '/', influx_bin_game, dirs_exist_ok=True)





