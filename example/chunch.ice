z: @(f): @(x): x;
s: @(n): @(f): @(x): f(n(f)(x));
p: @(n): @(f): @(x): n(@(g): @(h): h(g(f)))(@(u): x)(@(u): u);

ADD: @(m, n): @(f): @(x): m(f)(n(f)(x));

TRUE: @(x, y): x;
FALSE: @(x, y): y;

IsZero: @(n): n(@(x): FALSE)(TRUE);

BoolAnd: @(x, y): x(delay y, FALSE);
BoolOr: @(x, y): x(TRUE, delay y);
BoolNot: @(x): x(FALSE, TRUE);

IfThenElse:
    @(cond, true_expr, false_expr):
        cond(delay true_expr, delay false_expr);

ADD2: @(x, y): IfThenElsen(IsZero(x), y, s(ADD2(p(x), y)));

EQ: @(x, y):
    IfThenElse(BoolAnd(IsZero(x), IsZero(y)),
        TRUE,
        IfThenElse(BoolOr(IsZero(x), IsZero(y)),
            FALSE,
            delay EQ(p(x), p(y))
        )
    );

one: s(z);
two: ADD(one, one);

println(EQ(two, ADD(one, one)) = TRUE);
println(EQ(two, ADD(two, two)) = TRUE);
println(EQ(two, ADD2(one, one)) = TRUE);
