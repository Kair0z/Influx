import shutil, os, argparse
import re
from pathlib import Path
import subprocess

parser = argparse.ArgumentParser(description='parser')

parser.add_argument('--config', action="store", dest='config', default='')
parser.add_argument('--game', action="store", dest='game', default='')
parser.add_argument('--deps', nargs='*')
args = parser.parse_args()

influx_root = "../"
influx_bin = influx_root + "/bin/debug-windows-x86_64/"
influx_source = influx_root + "/source/"
influx_source_renderer = influx_source + "/influx/influx_renderer/"
influx_source_renderer_shaders = influx_source_renderer + "/shaders/"
influx_renderer_shader_manifest = influx_source_renderer_shaders + "/embedded_shaders.h"
influx_renderer_shader_source = influx_source_renderer_shaders + "/source/"
influx_shadercompiler_exe = influx_bin + "/tool_shadercompiler/tool_shadercompiler.exe"

# parse the embedded_shaders manifest file 
manifest_path = Path(influx_renderer_shader_manifest)
pattern = re.compile(r'#include\s+"([^"]+)"')
shader_names = []
with manifest_path.open("r", encoding="utf-8") as f:
    for line in f:
        match = pattern.search(line)
        if match:
            include_path = Path(match.group(1))
            name = include_path.name.removesuffix(".cso.inc")
            shader_names.append(name)

# for each shader, run shader compiler & generate .cso + .cso.inc file
# .cso is pure binary blob, whilst the .inc file is includable to embed inside your C++ binary
shader_include_folder = influx_source_renderer_shaders
for shader in shader_names:
    filename, shader_descriptor = shader.split("_", 1)
    full_suffix = "_" + shader_descriptor
    output_filename = filename + full_suffix + ".cso"
    entrypoint, shader_type = shader_descriptor.rsplit("-", 1)

    shader_input_file = influx_source_renderer_shaders + "/source/" + filename + ".hlsl"
    shader_output_file = influx_source_renderer_shaders + "/compiled/" + output_filename
    print("compiling:" + shader_descriptor + " -> /compiled/" + output_filename)

    # run shader compiler
    args = [
        influx_shadercompiler_exe,
        "+cv_inputpath " + shader_input_file,
        "+cv_outputpath " + shader_output_file,
        "+cv_entry " + entrypoint,
        "+cv_includes " + shader_include_folder
    ]
    
    # print(" ".join(f'"{c}"' if " " in c else c for c in args))
    proc = subprocess.Popen(
        args,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,  # decode bytes -> string
    )
    # Print stdout in real-time
    for line in proc.stdout:
        print("[]", line, end="")
    for line in proc.stderr:
        print("[]", line, end="")
    proc.wait()

    # run XXD to convert binary compile result blob into an includable .inc file
    shader_output_inc_file = shader_output_file + ".inc"
    shader_output_inc_path = Path(shader_output_inc_file)
    with shader_output_inc_path.open("w", encoding="utf-8", newline="\n") as out:
        xxdcmd = [
            "xxd",
            "-i",
            str(shader_output_file),
        ]
        # rint(" ".join(xxdcmd))
        subprocess.run(
            xxdcmd,
            stdout=out,
            stderr=subprocess.PIPE,
            check=True,
            text=True
        )

        # fix xxd symbol names
        text = shader_output_inc_path.read_text(encoding="utf-8")
        m = re.search(r'unsigned char\s+(\w+)\[\]', text)
        if not m:
            raise RuntimeError("xxd symbol not found")

        new_symbol_name = filename + "_" + entrypoint + "_cso"
        old = m.group(1)
        text = text.replace(old, new_symbol_name)
        text = text.replace(f"{old}_len", f"{new_symbol_name}_size")
        shader_output_inc_path.write_text(text, encoding="utf-8", newline="\n")


    

