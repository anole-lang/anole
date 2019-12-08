```
# delay expr
@Y(f):
  (@(x): f(delay x(delay x)))
  (@(x): f(delay x(delay x)))

@fact(f):
  @(n):
    if n = 0
    {
      return 1
    }
    else
    {
      retrun n * f(n-1)
    }

@result: Y(f)(5)
```
