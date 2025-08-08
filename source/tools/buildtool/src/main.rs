// petgraph
use petgraph::graph::{Graph, NodeIndex};
use petgraph::graphmap::DiGraphMap;
use petgraph::visit::Dfs;

// serde
use serde::Deserialize;

// walkdir
use walkdir::WalkDir;

// stdlib
use std::fs;
use std::collections::HashMap;
use std::path::Path;
use std::result::Result::{Ok, Err};

// vsproject
use vsproj::project::{Configuration, Project};

// toml
use toml;

macro_rules! default_string {
    ($name:ident, $value:expr) => {
        fn $name() -> String {
            $value.to_string()
        }
    };
}
default_string!(default_version     , "latest");
default_string!(default_description , "");
default_string!(default_kind        , "executable");
default_string!(default_author      , "unknown");
default_string!(default_cpp_standard, "C++20");
default_string!(default_csh_standard, "unknown");

#[derive(Deserialize, Debug)]
struct ProjectManifest {
    project: ProjectSection,
    language: LanguageSection,
}
#[derive(Deserialize, Debug)]
struct ProjectSection {
    name                    : String,
    #[serde(default = "default_version")]
    version                 : String,
    #[serde(default = "default_description")]
    description             : String,
    #[serde(default = "default_kind")]
    kind                    : String,
    #[serde(default)]
    authors                 : Vec<String>,
    #[serde(default)]
    dependencies: HashMap<String, String>,
}
#[derive(Deserialize, Debug)]
struct LanguageSection {
    cpp: CppSection,
}
#[derive(Deserialize, Debug)]
struct CppSection {
    #[serde(default = "default_cpp_standard")]
    standard: String,
}
#[derive(Deserialize, Debug)]
struct CsharpSection {
    #[serde(default = "default_csh_standard")]
    standard: String,
}

fn parse_and_print_manifest(path: &Path) -> Result<(), Box<dyn std::error::Error>> {
    let content = fs::read_to_string(path)?;
    let manifest: ProjectManifest = toml::from_str(&content)?;
    println!("Parsed manifest from {}:\n{:#?}", path.display(), manifest);
    Ok(())
}

fn generate_solution(projects: &[ProjectManifest]) -> String {
    

    let mut project = Project::new("influx_graphics", "path/to/influx_graphics");

    
    project.add_source("src/main.cpp");
    project.add_include_dir("include");
    project.set_kind("Application"); // or "StaticLibrary", etc.

    project.add_configuration(Configuration::debug());
    project.add_configuration(Configuration::release());

    project.write().unwrap();
}

fn clear_console() {
    // Works on both Windows and Unix-like systems
    if cfg!(target_os = "windows") {
        std::process::Command::new("cmd")
            .args(["/C", "cls"])
            .status()
            .unwrap();
    } else {
        std::process::Command::new("clear")
            .status()
            .unwrap();
    }
}

fn main() {
    clear_console();

    let source_folder = "D:/Git/Influx/source"

    // gather all toml files in influx source
    let toml_files: Vec<_> = WalkDir::new(source_folder)
        .into_iter()
        .filter_map(Result::ok)
        .filter(|e| {
            e.path()
            .extension()
            .map_or(false, |ext| ext == "toml")
        })
        .map(|e| e.into_path())
        .collect();

    // 2. You can now parse each manifest
    for path in &toml_files {
        if let Err(e) = parse_and_print_manifest(path) {
            eprintln!("Error parsing TOML: {}", e);
        }
    }

    // 2. make dep-graph

    // 3. make vs sln
}
