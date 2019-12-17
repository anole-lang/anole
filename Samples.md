###### Declare local variable or function

```
@var: 1
@foo(): none
@foo()
{
  return none
}
```

###### Assignment
```
a: 1
b: 2
b: a: 3
@foo(): a: 4  # it will assign the global a to 4
```

###### Closure

```
@adddd: @(a): @(b): @(c): @(d): a + b + c + d
adddd(1)(2)(3)(4)
```

###### Delay expr

```
@Y(f):
  (@(x): f(delay x(delay x)))
  (@(x): f(delay x(delay x)));

@fact(f):
  @(n): n ? (n * f(n-1)) 1;

@result: Y(fact)(5);
```
