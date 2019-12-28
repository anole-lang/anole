println("Let's start!");

@Y(f):
  (@(x): f(delay x(delay x)))
  (@(x): f(delay x(delay x)));

@fact(f):
  @(n): n ? (n * f(n-1)) , 1;

@fib(f):
  @(n): (n < 2) ? 1, f(n - 1) + f(n - 2);

#start: time();
#println(Y(fib)(20));
#print(time() - start);
#println("s");

start: time();
a: 1;
while a < 1000000, a: a + 1;
print(time() - start);
println("s");
