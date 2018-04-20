# Ice
> this is a implementation that almost by myself in this branch.

A funny language named Ice (an exercise for Compilers).

### Progress

###### Comments
```ice
# this is a comment
```

###### Variable Declaration
```ice
@a: 1
@b: a
```

###### Binary Operator
```ice
# support + - * / % = != <= > >=
@a: 1 + 1
@b: 4 - 1
@c: (a + b) - a * b / a % b # support ()
1 = 1 # 1
1 != 1 # 0
a <= 3 # 1
```

### TODO
* Complete Function Declaration & MethodCall
