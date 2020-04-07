use * from coroutine

@foo() {
    @i: 0;
    while i < 5 {
        println(i: i + 1);
        co_yield();
    }
}

@id1: co_create(foo);
@id2: co_create(foo);

@i: 0;
while i < 5 {
    co_resume(id1);
    co_resume(id2);
    i: i + 1;
}

co_destroy(id1);
co_destroy(id2);
