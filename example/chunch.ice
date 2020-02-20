z: @() {};
s: @(x): @(): x;

TRUE: @(x, y): x;
FALSE: @(x, y): y;

BoolAnd: @(x, y): x(delay y, FALSE);
BoolOr: @(x, y): x(TRUE, delay y);
BoolNot: @(x): x(FALSE, TRUE);

IfThenElse: @(cond, true_expr, false_expr): cond(delay true_expr, delay false_expr);

IsZero: @(x): x = z ? TRUE, FALSE;

ADD: @(x, y): IfThenElse(IsZero(x), y, delay s(ADD(x(), y)));

EQ: @(x, y):
    IfThenElse(BoolAnd(IsZero(x), IsZero(y)),
        TRUE,
        IfThenElse(BoolOr(IsZero(x), IsZero(y)),
            FALSE,
            delay EQ(x(), y())
        )
    );

one: s(z);
two: s(one);

println(EQ(two, ADD(one, one)) = TRUE);
