use std::fs;
use std::io::{self, IsTerminal, Read, Write};
use std::path::PathBuf;

use anole::{Interpreter, Parser, VERSION_LITERAL};

fn main() {
    if let Err(message) = run() {
        eprintln!("{message}");
    }
}

fn run() -> Result<(), String> {
    let arguments: Vec<_> = std::env::args_os().skip(1).collect();
    let file_index = arguments.iter().position(|argument| {
        let argument = argument.to_string_lossy();
        argument != "-r" && argument != "--version" && !argument.starts_with('-')
    });
    let option_boundary = file_index.unwrap_or(arguments.len());
    if arguments[..option_boundary]
        .iter()
        .any(|argument| argument == "--version")
    {
        println!("Anole {VERSION_LITERAL}");
        return Ok(());
    }

    if let Some(file_index) = file_index {
        let file = &arguments[file_index];
        let mut path = PathBuf::from(file);
        if path
            .extension()
            .is_none_or(|extension| extension != "anole")
        {
            path.push("__init__.anole");
        }
        let source = fs::read_to_string(&path)
            .map_err(|_| format!("anole: cannot open file {}", path.display()))?;
        let script_arguments = arguments[file_index..]
            .iter()
            .map(|argument| argument.to_string_lossy().into_owned())
            .collect();
        let output = Interpreter::with_arguments(script_arguments)
            .run(&source, &path.display().to_string())
            .map_err(|error| error.to_string())?;
        print!("{output}");
        if arguments[..file_index]
            .iter()
            .any(|argument| argument == "-r")
        {
            let ast = Parser::new(&source, path.display().to_string())
                .and_then(Parser::parse)
                .map_err(|error| error.to_string())?;
            let rd_path = PathBuf::from(format!("{}.rd", path.display()));
            fs::write(rd_path, format!("{ast:#?}\n")).map_err(|error| error.to_string())?;
        }
        return Ok(());
    }

    if !arguments.is_empty() {
        println!("invalid command-line argument(s)");
        return Ok(());
    }

    if io::stdin().is_terminal() {
        repl()
    } else {
        let mut source = String::new();
        io::stdin()
            .read_to_string(&mut source)
            .map_err(|error| error.to_string())?;
        let output = Interpreter::new()
            .run(&source, "<stdin>")
            .map_err(|error| error.to_string())?;
        print!("{output}");
        Ok(())
    }
}

fn repl() -> Result<(), String> {
    println!(
        "    _                _\n   / \\   _ __   ___ | | ___\n  / _ \\ | '_ \\ / _ \\| |/ _ \\\n / ___ \\| | | | (_) | |  __/   {VERSION_LITERAL}\n/_/   \\_\\_| |_|\\___/|_|\\___|\n"
    );
    let mut interpreter = Interpreter::new();
    loop {
        let Some(source) = read_repl_source()? else {
            return Ok(());
        };
        if source.trim().is_empty() {
            continue;
        }
        match interpreter.run_repl(&source) {
            Ok(output) => print!("{output}"),
            Err(error) => eprintln!("{error}"),
        }
        if interpreter.is_halted() {
            return Ok(());
        }
    }
}

fn read_repl_source() -> Result<Option<String>, String> {
    let mut source = String::new();
    loop {
        let prompt = if source.is_empty() { ">> " } else { ".. " };
        print!("{prompt}");
        io::stdout().flush().map_err(|error| error.to_string())?;
        let mut line = String::new();
        if io::stdin()
            .read_line(&mut line)
            .map_err(|error| error.to_string())?
            == 0
        {
            return Ok((!source.is_empty()).then_some(source));
        }
        source.push_str(&line);
        if delimiters_balanced(&source) {
            return Ok(Some(source));
        }
    }
}

fn delimiters_balanced(source: &str) -> bool {
    let mut stack = Vec::new();
    let mut in_string = false;
    let mut escaped = false;
    for character in source.chars() {
        if in_string {
            if escaped {
                escaped = false;
            } else if character == '\\' {
                escaped = true;
            } else if character == '"' {
                in_string = false;
            }
            continue;
        }
        match character {
            '"' => in_string = true,
            '(' | '[' | '{' => stack.push(character),
            ')' if stack.pop() != Some('(') => return true,
            ']' if stack.pop() != Some('[') => return true,
            '}' if stack.pop() != Some('{') => return true,
            _ => {}
        }
    }
    stack.is_empty() && !in_string
}
