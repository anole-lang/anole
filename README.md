# Ice
A funny language named Ice.

### Progress

###### Comments
```ice
# this is a comment
```

###### Variable Declaration
```ice
a : 1
b : a
```

###### Arithmetic Operator
```ice
# support + - * / %
a : 1 + 1
b : 2 - 1
c : (a + b) - a * b / a # also support ()
```

###### Print
```ice
a : 1
b : a + 3 * 4 - 2 / 2 + 2 % 4
print(b) / 2 # output: 7
```

###### Function Declaration & MethodCall
```ice
# decl_1
func(arg1, arg2, ...) : return_value

# decl_2
func(arg1, arg2, ...) : return_value
{
    stmt
    ...
}

# call
func(expr1, expr2, ...)

# example
add(a, b) : a + b
print(add(1, 2))
```


### TODO
* Fix the Bug in Function Declaration