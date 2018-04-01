# Ice
> this is a implementation that almost by myself in this branch.

A funny language named Ice ( and only supports integer operations now ).

### Progress

###### Comments
```ice
# this is a comment
```

###### Variable Declaration
```ice
a: 1
b: a
```

###### Binary Operator
```ice
# support + - * / % = != < <= > >=
a: 1 + 1
b: 2 - 1
c: (a + b) - a * b / a # also support ()
```

###### Print
```ice
a: 1
b: a + 3 * 4 - 2 / 2 + 2 % 4
print(b) / 2 # output: 7
```

###### Function Declaration & MethodCall
```ice
# decl_1
@func(arg1, arg2, ...): return_value

# decl_2
@func(arg1, arg2, ...)
{
    stmt
    ...
    return return_value
}

# call
func(expr1, expr2, ...)

# example
@add(a, b): a + b

@sum(a, b, c)
{
    return a + b + c
}

print(add(1, 2))
print(sum(1, 2, 3))
```

###### Control Flow
```ice
@max(a, b)
{
    if a < b
    {
        res: b
    }
    else
    {
        res: a
    }
    return res
}
print(max(1, 2))
```
### TODO
* Lexical analyzer
