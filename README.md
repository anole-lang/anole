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
