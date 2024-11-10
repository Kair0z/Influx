import subprocess, sys, os

REQUIRED_PYTHON = (3, 8)
if sys.version_info < REQUIRED_PYTHON:
    sys.exit(f"Python {REQUIRED_PYTHON[0]}.{REQUIRED_PYTHON[1]} or above is required. You are using Python {sys.version_info.major}.{sys.version_info.minor}.")

args = sys.argv
python_executable = sys.executable
args = [python_executable] + args

args[1] = os.getcwd() + "\stage_content.py"
subprocess.run(args)

args[1] = os.getcwd() + "\stage_binaries.py"
subprocess.run(args)