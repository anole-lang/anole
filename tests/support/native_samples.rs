pub(crate) struct NativeSample {
    pub(crate) name: &'static str,
    pub(crate) source: &'static str,
    pub(crate) output: &'static str,
}

// Fixed behavioral samples for the Anole runtime.
pub(crate) const NATIVE_SAMPLES: &[NativeSample] = &[
    NativeSample {
        name: "SimpleRun",
        source: r#"
a: 1;
b: 2;
b: a : 3;
print(a + b);"#,
        output: "6",
    },
    NativeSample {
        name: "SimpleFunction",
        source: r#"
@adddd: @(a): @(b): @(c): @(d): a + b + c + d;
print(adddd(1)(2)(3)(4));"#,
        output: "10",
    },
    NativeSample {
        name: "VariadicArguments",
        source: r#"
@foo(a, b, ...c) {
    return c;
}

@l: [2, 3, 4, 5, 6];

println(foo(1, l..., 7));"#,
        output: "[3, 4, 5, 6, 7]\n",
    },
    NativeSample {
        name: "SimpleIfElseStmt",
        source: r#"
a: 1;
@foo(x) {
    a : 1;
    if x {
        return a: 2;
    } else {
        return a: 3;
    };
};
print(foo(1));
print(foo(0));"#,
        output: "23",
    },
    NativeSample {
        name: "Y",
        source: r#"
@Y(f):
  (@(x): f(delay x(delay x)))
  (@(x): f(delay x(delay x)));

@fact(f):
  @(n): n ? (n * f(n-1)) , 1;

print(Y(fact)(5));"#,
        output: "120",
    },
    NativeSample {
        name: "Chunch",
        source: r#"
Zero: @(f): @(x): x;
Succ: @(n): @(f): @(x): f(n(f)(x));
Pred: @(n): @(f): @(x): n(@(g): @(h): h(g(f)))(@(u): x)(@(u): u);

Plus: @(m, n): @(f): @(x): m(f)(n(f)(x));
Mult: @(m, n): @(f): n(m(f));
Exp: @(m, n): n(m);

True: @(x, y): x;
False: @(x, y): y;

IsZero: @(n): n(@(x): False)(True);

BoolAnd: @(x, y): x(y, False);
BoolOr: @(x, y): x(True, y);
BoolNot: @(x): x(False, True);

IfThenElse:
    @(cond, true_expr, false_expr):
        cond(delay true_expr, delay false_expr);

Add2: @(x, y): IfThenElse(IsZero(x), y, delay Succ(Add2(Pred(x), y)));

Equal: @(x, y):
    IfThenElse(BoolAnd(IsZero(x), IsZero(y)),
        True,
        IfThenElse(BoolOr(IsZero(x), IsZero(y)),
            False,
            delay Equal(Pred(x), Pred(y))));

One: Succ(Zero);
Two: Plus(One, One);
Four: Mult(Two, Two);

println(Equal(Two, Plus(One, One)) = True);
println(Equal(Two, Plus(Two, Two)) = True);
println(Equal(Two, Add2(One, One)) = True);

@show(x) {
    println(x);
    return x;
}

One(show)(1);
Two(show)(2);
Four(show)(4);"#,
        output: "true\nfalse\ntrue\n1\n2\n2\n4\n4\n4\n4\n",
    },
    NativeSample {
        name: "Continuation",
        source: r#"
Xb: @{
    println("Hi! My name is Xu Bo.");
    cont: call_with_current_continuation(Lyx);
    println("Do you love me?");
    call_with_current_continuation(cont);
}

Lyx: @(cont) {
    println("Hello! I'm Luo yuexuan.");
    cont: call_with_current_continuation(cont);
    println("Yes! I love you very very much!");
    cont(none);
}

Xb();
"#,
        output: concat!(
            "Hi! My name is Xu Bo.\n",
            "Hello! I'm Luo yuexuan.\n",
            "Do you love me?\n",
            "Yes! I love you very very much!\n",
        ),
    },
    NativeSample {
        name: "CostumTryCatch",
        source: r#"
@Except: {
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

    @catch(try, cfun) {
        @e: call_with_current_continuation(@(cont) {
            conts.push(cont);
            try(@(x): x)();
        });
        if !(e is none) {
            cfun(e);
        }
    }

    @try(fun): @(f): f(fun);

    return @{};
};

@throw: Except.throw;
prefixop throw;

@try: Except.try;
prefixop try;

@catch: Except.catch;
infixop catch;

@div(a, b) {
    if b = 0 {
        throw "err: div 0";
    }
    return a / b;
};

@div_forever(a) {
    @b: a;
    while true {
        div(a, b);
        b: b - 1;
    }
};

try @{
    div_forever(100);
}
catch @(e) {
    println(e);
}"#,
        output: "err: div 0\n",
    },
    NativeSample {
        name: "CustomOp",
        source: r#"
@*=*(lhs, rhs): lhs + rhs;

@*^*(lhs, rhs) {
    return lhs + rhs;
}

infixop 50  *=*;
infixop 200 *^*;

println(2 * 3 *=* 4 * 5);
println(2 * 3 *^* 4 * 5);

@refof(&var): delay var;
prefixop refof;

@a: 1;
@test(var): var: 10;
test(refof a);
println(a);"#,
        output: "26\n70\n10\n",
    },
    NativeSample {
        name: "DefaultArgument",
        source: r#"
foo: @(m, n: n) {
    return m + n;
};

n: 1;
println(foo(1));

n: 2;
println(foo(1));

foo: @(x, func: @(x): x + y) {
    return func(x);
};

y: 10;
println(foo(10));

println(foo(10, @(x): x + 1));"#,
        output: "2\n3\n20\n11\n",
    },
    NativeSample {
        name: "SimpleEnum",
        source: r#"
State: enum {
    Start,
    Running,
    End
};

state: State.Start;
while (state != State.End) {
    println(match state {
        State.Start => {
            state: State.Running;
            return "start";
        },
        State.Running => {
            state: State.End;
            return "running"
        },
        => "end"
    });
}"#,
        output: "start\nrunning\n",
    },
    NativeSample {
        name: "SimpleClass",
        source: r#"
@Student: class {
    age: 10;
    __init__(self) {
        self.name: "unknown";
    };
};

stu: Student();
println(Student.age);
println(stu.age);
println(stu.name);
stu.age: 20;
println(Student.age);
println(stu.age);
"#,
        output: "10\n10\nunknown\n10\n20\n",
    },
    NativeSample {
        name: "NestedForeach",
        source: r#"
foreach [1, 2] as i {
    foreach [2, 3] as j {
        print(i);
        print(j);
    }
}
"#,
        output: "12132223",
    },
    NativeSample {
        name: "MultiDeclaration",
        source: r#"
@prints(...args) {
    foreach args as arg {
        print(arg);
        print(" ");
    }
    // print("\n");
}

@a, b: 1, 2;
prints(a, b);
@a, b: [3, 4];
prints(a, b);
@[a, b]: [5, 6];
prints(a, b);
"#,
        output: "1 2 3 4 5 6 ",
    },
];
