Except: (@() {
    @conts: [];

    @throw(e) {
        if conts.empty() {
            println(e);
            exit();
        } else {
            @cont: conts.pop();
            cont(e);
        }
    }

    @try(f, catch) {
        @e: call_with_current_continuation(@(cont) {
            conts.push(cont);
            f();
        });
        if !(e is none), catch(e);
    }

    return @() {
        throw; try;
    };
})();

div: @(a, b) {
    if b = 0, Except.throw("err: div 0");
    return a / b;
}

div_forever: @(a) {
    @b: a;
    while true {
        div(a, b);
        b: b - 1;
    }
}

test: @() {
    Except.try (@() {
        div_forever(100);
    }, @(e) {
        println(e);
    });
}

test();
