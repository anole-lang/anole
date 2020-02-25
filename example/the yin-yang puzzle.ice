@yin: (@(cc) { print("@"); return cc; })
    (call_with_current_continuation(@(cont): cont));

@yang: (@(cc) { print("*"); return cc; })
    (call_with_current_continuation(@(cont): cont));

yin(yang);
