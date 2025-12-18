import shutil, os, argparse
import re
from pathlib import Path
import subprocess

influx_root = "../"
influx_bin = influx_root + "/bin/debug-windows-x86_64/"
influx_source = influx_root + "/source/"
influx_source_renderer = influx_source + "/influx/influx_renderer/"
influx_renderer_shaders = influx_source_renderer + "/shaders/"
influx_renderer_shader_manifest = influx_renderer_shaders + "/embedded_shaders.h"
influx_renderer_shader_source = influx_renderer_shaders + "/source/slang/"
influx_renderer_shader_compiled = influx_renderer_shaders + "/compiled/"
influx_shadercompiler_exe = influx_bin + "/tool_shadercompiler/tool_shadercompiler.exe"

def merge_files(file_a, file_b):
    with open(file_a, "a") as f1, open(file_b, "r") as f2:
        f1.write(f2.read())
        
    # file_b.close()
    os.remove(file_b)

def make_includable_file(source_bin_filepath, dest_inc_filepath, filename, entrypoint, postfix):
    with dest_inc_filepath.open("w", encoding="utf-8", newline="\n") as out:
        xxdcmd = [ "xxd", "-i", str(source_bin_filepath) ]
        # print(" ".join(xxdcmd))
        subprocess.run(
            xxdcmd,
            stdout=out,
            stderr=subprocess.PIPE,
            check=True,
            text=True
        )
        
        # fix xxd symbol names
        text = dest_inc_filepath.read_text(encoding="utf-8")
        m = re.search(r'unsigned char\s+(\w+)\[\]', text)
        if not m:
            raise RuntimeError("xxd symbol not found")
        
        new_symbol_name = filename + "_" + entrypoint + postfix 
        old = m.group(1)
        text = text.replace(old, new_symbol_name)
        text = text.replace(f"{old}_len", f"{new_symbol_name}_size")
        dest_inc_filepath.write_text(text, encoding="utf-8", newline="\n")

def main():
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
    for shader in shader_names:
        filename, shader_descriptor = shader.split("_", 1)
        full_suffix = "_" + shader_descriptor
        output_filename = filename + full_suffix + ".cso"
        entrypoint, shader_type = shader_descriptor.rsplit("-", 1)

        # input: shader.slang
        # output: /compiled/shader_vs
        shader_input_file = influx_renderer_shader_source + filename + ".slang"
        shader_output_file = influx_renderer_shader_compiled + output_filename
        shader_refl_file = shader_output_file + ".refl" 
        print("compiling:" + shader_descriptor + " -> /compiled/" + output_filename)

        # run shader compiler
        args = [
            influx_shadercompiler_exe,
            "+cv_inputpath " + shader_input_file,
            "+cv_outputpath " + shader_output_file,
            "+cv_reflpath" + shader_refl_file,
            "+cv_entry " + entrypoint,
            "+cv_includes " + influx_renderer_shader_source
        ]

        # print(" ".join(f'"{c}"' if " " in c else c for c in args))
        # print(" ".join(args))
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
        compile_result = proc.wait()

        # run XXD to convert binary compile result blob into an includable .inc file
        if compile_result == 0:
            out_include_file = shader_output_file + ".inc" # .cso.inc
            out_refl_file = shader_refl_file + ".inc" ## .rootsignature.inc
            make_includable_file(shader_output_file, Path(out_include_file), filename, entrypoint, "_cso")
            make_includable_file(shader_output_file, Path(out_refl_file), filename, entrypoint, "_refl")
            merge_files(Path(out_include_file), Path(out_refl_file))

if __name__ == "__main__":
    main()

