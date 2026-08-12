#[path = "support/compiletest.rs"]
mod compiletest;

fn main() {
    let mut arguments = std::env::args().skip(1);
    let mut bless = false;
    let mut filter = None;

    while let Some(argument) = arguments.next() {
        match argument.as_str() {
            "--bless" => bless = true,
            "--filter" => {
                filter = Some(
                    arguments
                        .next()
                        .unwrap_or_else(|| panic!("--filter requires a value")),
                );
            }
            "--help" | "-h" => {
                print_help();
                return;
            }
            _ if argument.starts_with("--filter=") => {
                filter = Some(argument["--filter=".len()..].to_owned());
            }
            _ => panic!("unknown compile test argument {argument:?}; use --help for usage"),
        }
    }

    compiletest::run(bless, filter.as_deref());
}

fn print_help() {
    println!(
        "Anole compile tests\n\n\
         Usage: cargo test --test compile_tests -- [OPTIONS]\n\n\
         Options:\n\
           --bless            Update expected .result/.stdout/.stderr files\n\
           --filter <TEXT>    Run cases whose path contains TEXT\n\
           -h, --help         Print help"
    );
}
