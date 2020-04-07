use _open from libfileobject;

@mode: (@(){
    @app:    1 << 0;
    @binary: 1 << 1;
    @in:     1 << 2;
    @out:    1 << 3;
    @trunc:  1 << 4;
    @ate:    1 << 5;

    return @() {};
})();

@open(path, mode: mode.in | mode.out) {
    return _open(path, mode);
}
