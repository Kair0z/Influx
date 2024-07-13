import subprocess, sys, os

args = sys.argv
python_executable = sys.executable
args = [python_executable] + args

args[1] = os.getcwd() + "\stage_content.py"
subprocess.run(args)

args[1] = os.getcwd() + "\stage_binaries.py"
subprocess.run(args)