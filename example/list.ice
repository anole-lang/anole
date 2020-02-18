nil: @() {};

list:
  @(head, tail):
    @(f):
      f(head, tail);

list_head:
  @(l):
    l(@(head, tail): head);

list_tail:
  @(l):
    l(@(head, tail): tail);

list_last:
  @(l):
    list_tail(l) = nil ? list_head(l), list_last(list_tail(l));

list_size:
  @(l):
    list_tail(l) = nil ? 1, 1 + list_size(list_tail(l));

list_append:
  @(l, item):
    list(list_head(l), list_tail(l) = nil ? list(item, nil), list_append(list_tail(l), item));

l: list(1, nil);
println(list_size(l));
l: list_append(l, 2);
println(list_size(l));
l: list_append(l, 3);
println(list_size(l));

println(list_head(l));
println(list_head(list_tail(l)));
println(list_last(l));
