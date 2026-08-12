#[path = "support/native_samples.rs"]
mod native_samples;

use anole::Interpreter;

#[test]
fn executes_every_native_runtime_sample() {
    for sample in native_samples::NATIVE_SAMPLES {
        let output = Interpreter::new()
            .run(sample.source, "<test>")
            .unwrap_or_else(|error| panic!("native sample {} failed: {error}", sample.name));
        assert_eq!(output, sample.output, "native sample {}", sample.name);
    }
}
