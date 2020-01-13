println("Let's start!");

@fib1(n): (n < 2) ? 1, fib1(n - 1) + fib1(n - 2);

@Y(f):
  (@(x): f(delay x(delay x)))
  (@(x): f(delay x(delay x)));

@fib2(f):
  @(n): (n < 2) ? 1, f(n - 1) + f(n - 2);


start: time();
fib1(30);
print(time() - start);
println("s");

start: time();
Y(fib2)(30);
print(time() - start);
println("s");

start: time();
a: 1;
while a < 10000000, a: a + 1;
print(time() - start);
println("s");
