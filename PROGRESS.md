Please add feature request here.

<hr>

### Features
+ [X] Anonymous Function
+ [X] Match Expression

<hr>

### TODO
+ [ ] Support for method().member / method().member()
+ [ ] Fix Bugs 
+ [ ] Refactoring

<hr>

### ChangeLog
+ [X] Fixed bug for objects built in class block when the class was instantiated
+ [X] Dict type & built-in methods
+ [X] Simple garbage collection (reference counting)
+ [X] Fixed bug with scope when using '.' operator
+ [X] List type & built-in methods
+ [X] Match expression
+ [X] Enum expression
+ [X] Simple class definition
+ [X] Anonymous function
+ [X] Common control flow
+ [X] Function definition
+ [X] Operators that act on objects
+ [X] Parsing integer, float, and string
+ [X] Filter out comments

<hr>

### Examples

###### Comments
```ice
# hello world
```

###### Control Flow
```ice
# support if_else while do_while for break continue

@fib(n)
{
    if (n = 0) + (n = 1) { return 1 }
    else { return fib(n-1) + fib(n-2) }
}

fib(10) # 89

@a: 3
while a
{
    print(a)
    @a: a - 1
}

@a: 0
do {
    @a: a + 1
    if a = 3 { break }
    print(a)
} while a < 5

for 1 to 5
{
    @a: a + 1
    if a = 3 { continue }
    print(a)
}
```

###### Anonymous Function
```ice
@add(a, b): a + b
@mul: @(a, b){
    return a * b
}
@(a, b){ return a / b }(9, 3)

@quadraticSum: @(a, b){
    @sqrt: @(n){ return n * n }
    return @(a, b){ return a + b }(sqrt(a), sqrt(b))
}
```

###### Using Module & Simple OOP
```ice
# demo.ice
@@Math()
{
    @add(a, b): a + b

    @mul: @(a, b){
        return a * b
    }

    @quadraticSum: @(a, b){
        @sqrt: @(n){ return n * n }
        return @(a, b){ return a + b }(sqrt(a), sqrt(b))
    }

    @Math(base){
        @self.base: base
    }
}
```

```ice
# run.ice
using demo

@math: new Math(10)

math.quadraticSum(3, 4)

@math.pow(num, n)
{
    @res: 1
    while n > 0
    {
        @res: res * num
        @n: n - 1
    }
    return res
}

math.pow(5, 3)
```

###### Enum Expression
```ice
@TOKEN: { TBEGIN, TEND }
TOKEN.TBEGIN
```

###### Match Expression
```ice
@numeber: 1 # or 2, 3, 4, ...
@name: match numeber{
        1 => "one",
        2 => "two",
        3 => "three"
    } else "other"
```

###### List Type
```ice
@list: [1, 2, 3]
@list[0]: [1, 2, 3]
```

###### Dict Type
```ice
@dict: {1: 2, "cyy": "TianCaiChengXuYuan"}
@dict["cyy"]
```