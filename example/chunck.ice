zero: @() {};
suc: @(x): @(): x;
add: @(m, n): (m = zero) ? n, suc(add(m(), n));
eq: @(m, n) {
    if m = zero and n = zero, return true;
    elif m = zero or n = zero, return false;
    else, return eq(m(), n());
}
one: suc(zero);
two: add(one, one);
println(eq(two, add(one, one)));
