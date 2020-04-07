use _open from libfileobject;

@base: (@(){
    @app: 1 << 0;
    @binary: 1 << 1;
    @in: 1 << 2;
    @out: 1 << 3;
    @trunc: 1 << 4;
    @ate: 1 << 5;

    return @() {};
})();

@open(path, mode: base.in | base.out) {
    return _open(path, mode);
}
