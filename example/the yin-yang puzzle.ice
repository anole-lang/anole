(@(yang): @(yin): yin(yang))
    ((@(cont) { print("*"); return cont; })
        (call_with_current_continuation(@(cont): cont)))
    ((@(cont) { print("@"); return cont; })
        (call_with_current_continuation(@(cont): cont)));
