use std::env;
use std::path::PathBuf;
use std::process::Command;

fn main() {
    let c_src_dir = PathBuf::from("../c_src");
    let build_dir = c_src_dir.join("build");
    let makefile_path = c_src_dir.join("Makefile");

    // Re-run build script if Makefile or C source files change
    println!("cargo:rerun-if-changed={}", makefile_path.display());

    // Run `make -C ../c_src`
    let status = Command::new("make")
        .arg("-C")
        .arg(&c_src_dir)
        .status()
        .expect("Failed to run `make`");

    if !status.success() {
        panic!("Makefile build failed");
    }

    // Tell cargo to link the compiled object files from ../c_src/build
    println!("cargo:rustc-link-search=native={}", build_dir.display());

    // You can use bindgen still, if needed:
    let header_path = c_src_dir.join("src/sealib.h"); // or whatever your main header is
    println!("cargo:rerun-if-changed={}", header_path.display());

    let bindings = bindgen::Builder::default()
        .header(header_path.to_str().unwrap())
        .parse_callbacks(Box::new(bindgen::CargoCallbacks::new()))
        //.no_std()
        .use_core()
        .generate()
        .expect("Unable to generate bindings");

    let out_path = PathBuf::from(env::var("OUT_DIR").unwrap()).join("bindings.rs");
    bindings
        .write_to_file(out_path)
        .expect("Couldn't write bindings!");

    // Link individual object files or a static lib from build dir
    // Example: If you have libfoo.a, tell cargo to link it
    //println!("cargo:rustc-link-lib=static=foo"); // looks for libfoo.a in build/
}
