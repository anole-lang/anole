use std::collections::BTreeSet;

#[test]
fn golden_files_cover_every_serialized_opcode() {
    let mut opcodes = BTreeSet::new();
    for fixture in [
        include_str!("fixtures/ir/basic.ir.hex"),
        include_str!("fixtures/ir/expressions.ir.hex"),
        include_str!("fixtures/ir/control.ir.hex"),
        include_str!("fixtures/ir/types.ir.hex"),
        include_str!("fixtures/ir/imports.ir.hex"),
        include_str!("fixtures/ir/dep.ir.hex"),
        include_str!("fixtures/ir/operators.ir.hex"),
    ] {
        collect_opcodes(&decode_hex(&fixture.replace('\n', "")), &mut opcodes);
    }

    // PlaceHolder (0) is patched before serialization. Every serialized opcode
    // from Pop (1) through BuildClass (51) must occur in the suite.
    assert_eq!(opcodes, (1_u8..=51).collect());
}

fn collect_opcodes(bytes: &[u8], opcodes: &mut BTreeSet<u8>) {
    assert!(bytes.len() >= 32);
    assert_eq!(read_u64(bytes, 0), 20_210_213);
    let constants = read_u64(bytes, 8) as usize;
    let instructions = read_u64(bytes, 16) as usize;
    let mut cursor = 32;
    for _ in 0..constants {
        let tag = bytes[cursor];
        cursor += 1;
        match tag {
            b'i' | b'f' => cursor += 8,
            b's' => cursor = skip_string(bytes, cursor),
            _ => panic!("unknown constant tag {tag}"),
        }
    }

    for _ in 0..instructions {
        let opcode = bytes[cursor];
        cursor += 1;
        opcodes.insert(opcode);
        match opcode {
            1 | 7 | 16 | 19 | 20 | 21 | 22 | 26 | 28 | 49 | 50 => cursor += 8,
            2 | 3 | 5 | 6 | 8 | 10 | 11 | 23 | 51 => cursor = skip_string(bytes, cursor),
            24 => {
                cursor = skip_string(bytes, cursor);
                cursor += 8;
            }
            27 => cursor += 16,
            _ => {}
        }
    }
}

fn skip_string(bytes: &[u8], cursor: usize) -> usize {
    cursor + 8 + read_u64(bytes, cursor) as usize
}

fn read_u64(bytes: &[u8], cursor: usize) -> u64 {
    u64::from_ne_bytes(bytes[cursor..cursor + 8].try_into().unwrap())
}

fn decode_hex(hex: &str) -> Vec<u8> {
    hex.as_bytes()
        .chunks_exact(2)
        .map(|pair| {
            let pair = std::str::from_utf8(pair).unwrap();
            u8::from_str_radix(pair, 16).unwrap()
        })
        .collect()
}
