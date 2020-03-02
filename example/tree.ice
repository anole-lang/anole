tip: @() {}

tree: @(left, val, right):
  @(f):
    f(left, val, right);

tree_init: @(val):
  tree(tip, val, tip)

bst_insert: @(bst, val):
  (bst = tip) ?
    tree_init(val),
    (bst(@(l, v, r): (v < val) ?
      tree(l, v, bst_insert(r, val)),
      tree(bst_insert(l, val), v, r)));

tree_infix: @(bst): bst(@(l, v, r) {
    tree_infix(l);
    print(v);
    print(" ");
    tree_infix(r);
})

t: tip;
a: 1;
while a < 10 {
    t: bst_insert(t, a);
    a: a + 1;
}

tree_infix(t);
