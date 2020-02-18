nil: @() {}

pair: @(x, y): @(z): z(x, y);
fst: @(p): p(@(x, y): x);
snd: @(p): p(@(x, y): y);

map:
  @(head, tail):
    @(f):
      f(head, tail);

map_head:
  @(m):
    m(@(head, tail): head);

map_tail:
  @(m):
    m(@(head, tail): tail);

map_insert:
  @(m, item):
    map_tail(m) = nil ?
      map(map_head(m), map(item, nil)),
      map(map_head(m), map_insert(map_tail, item));

map_at:
  @(m, k):
    map_tail(m) = nil ?
      (fst(map_head(m)) = k ? snd(map_head(m)), nil),
      map_at(map_tail(m), k);

m: map(pair(1, 2), nil);
println(map_at(m, 1));

m: map_insert(m, pair(3, 4));
println(map_at(m, 3));
