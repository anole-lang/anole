use _open from libfileobject;

@open(path, mode: "r") {
    return _open(path, mode);
}
