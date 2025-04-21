//pub fn add(left: u64, right: u64) -> u64 {
//left + right
//}

#[no_mangle]
pub extern "C" fn rust_function() -> i32 {
    42
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
