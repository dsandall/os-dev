//pub fn add(left: u64, right: u64) -> u64 {
//left + right
//}

#![no_std] // don't link the Rust standard library
#![no_main] // disable all Rust-level entry points

use core::panic::PanicInfo;

/// This function is called on panic.
#[panic_handler]
fn panic(_info: &PanicInfo) -> ! {
    loop {}
}

//#[unsafe(no_mangle)] // don't mangle the name of this function
//pub extern "C" fn _start() -> ! {
//    // this function is the entry point, since the linker looks for a function
//    // named `_start` by default
//    loop {}
//}

// import our C bindings
include!(concat!(env!("OUT_DIR"), "/bindings.rs"));

// Create a rust function that calls a C binding
#[no_mangle]
pub extern "C" fn rust_function() -> i32 {
    return unsafe { get78() };
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn it_works() {
        let result = rust_function();
        assert_eq!(result, 42);
    }
}
