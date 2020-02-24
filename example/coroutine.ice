coroutine: (@() {
    @funcs: [];
    @conts: [];
    @deletes: [];
    @co_cont: none;

    @create(func) {
        if deletes.empty() {
            @id: funcs.size();
            funcs.push(func);
            conts.push(none);
            return id;
        } else {
            @id: deletes.pop();
            funcs[id]: func;
            conts[id]: none;
            return id;
        }
    }

    @resume(id) {
        if conts[id] = none {
            conts[id]: call_with_current_continuation(@(cont) {
                co_cont: cont;
                funcs[id]();
            });
        } else {
            conts[id]: call_with_current_continuation(@(cont) {
                co_cont: cont;
                conts[id](none);
            });
        }
    }

    @yield() {
        call_with_current_continuation(co_cont);
    }

    @destroy(id) {
        funcs[id]: conts[id]: none;
        deletes.push(id);
    }

    return @() {};
})();

@foo() {
    @i: 0;
    while i < 5 {
        println(i: i + 1);
        coroutine.yield();
    }
}

@id1: coroutine.create(foo);
@id2: coroutine.create(foo);

@i: 0;
while i < 5 {
    coroutine.resume(id1);
    coroutine.resume(id2);
    i: i + 1;
}

coroutine.destroy(id1);
coroutine.destroy(id2);
