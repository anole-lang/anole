<div align=center>
<img width="300" height="300" src="https://images-1252918210.cos.ap-beijing.myqcloud.com/ICE_LOGO_1.png"/>
</div>
<hr>

## Just an exercise for *Compilers*.
> this is a implementation that almost by myself in this branch.

### Quick start
```shell
git clone https://github.com/MU001999/ice.git
cd ice/ice
make
```
Also you can use Visual Studio, just open ice/ice.sln by Visual Studio.

###### Hello World
```ice
print("Hello World")
```

### Progress

###### Comments
```ice
# this is a comment
```

###### Built-Functions
```ice
@a: input()
print(1)
str(10)
exit()
```

###### Variable Declaration
```ice
@a: 1
@b: a
```

###### Unary Operator
```ice
@a: "abc"
@b: -a # "cba"
```

###### Binary Operator
```ice
# support + - * / % = != <= > >=
@a: 1 + 1
@b: input()
@c: (a + b) - a * b / a % b # support ()
a = b
```

###### Function Declaration & Call
```ice
@add(a, b): a + b

@mul(a, b)
{
    return a * b
}

@pow(num, n)
{
    @res: 1
    while n > 0:
    {
        @res: res * num
        @n: n - 1
    }
    return res
}

mul(mul(2, 3), pow(3)) # 54
```

###### Control Flow
```ice
# support if_else while do_while for break continue

@fib(n)
{
    if (n = 0) + (n = 1)
    {
        return 1
    }
    else
    {
        return fib(n-1) + fib(n-2)
    }
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
    if a = 3
    {
        break
    }
    print(a)
} while a < 5

for 1 to 5
{
    @a: a + 1
    if a = 3
    {
        continue
    }
    print(a)
}
```

###### Lambda Expression
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

###### Interpret Single-File
```shell
/path/to/ice /path/to/source.ice
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
}
```

```ice
# run.ice
using demo

@math: new Math()
math.quadraticSum(3, 4)
```

### TODO
* Simple OOP
* Fix Bugs 
* Refactoring