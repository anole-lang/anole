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

println(foo(10, @(x): x + 1));
