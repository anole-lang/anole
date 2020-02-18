pair: @(x, y): @(z): z(x, y);
fst: @(p): p(@(x, y): x);
snd: @(p): p(@(x, y): y);

println(fst(pair(1, 2)));
