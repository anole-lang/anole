@_co_funcs: [];
@_co_conts: [];
@_co_deletes: [];
@_co_cont: none;

@co_create(func) {
    if _co_deletes.empty() {
        @id: _co_funcs.size();
        _co_funcs.push(func);
        _co_conts.push(none);
        return id;
    } else {
        @id: _co_deletes.pop();
        _co_funcs[id]: func;
        _co_conts[id]: none;
        return id;
    }
}

@co_resume(id) {
    if _co_conts[id] = none {
        _co_conts[id]: call_with_current_continuation(@(cont) {
            _co_cont: cont;
            _co_funcs[id]();
        });
    } else {
        _co_conts[id]: call_with_current_continuation(@(cont) {
            _co_cont: cont;
            _co_conts[id](none);
        });
    }
}

@co_yield() {
    call_with_current_continuation(_co_cont);
}

@co_destroy(id) {
    _co_funcs[id]: _co_conts[id]: none;
    _co_deletes.push(id);
}
