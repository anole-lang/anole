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
print b / 2 # output: 7
```

###### Function Declaration
```ice
# decl_1
func(arg, arg, ...) : return_value

# decl_2
func(arg, arg, ...) : return_value
{
    stmt
    ...
}
```


### TODO
* Function Declaration