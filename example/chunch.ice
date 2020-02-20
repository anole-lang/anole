Zero: @(f): @(x): x;
Succ: @(n): @(f): @(x): f(n(f)(x));
Pred: @(n): @(f): @(x): n(@(g): @(h): h(g(f)))(@(u): x)(@(u): u);

Add: @(m, n): @(f): @(x): m(f)(n(f)(x));

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
Two: Add(One, One);

println(Equal(Two, Add(One, One)) = True);
println(Equal(Two, Add(Two, Two)) = True);
println(Equal(Two, Add2(One, One)) = True);
