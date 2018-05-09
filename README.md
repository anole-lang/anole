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

<hr><hr><hr>

###### Comments
```ice
# this is a comment
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

@math: new Math()

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
@list: [{}, [{}], [1, 2, 3]], []]
@list[0].add(a, b): a + b
list[0].add(3, 4)
```

### TODO
* Fix Bugs 
* Refactoring