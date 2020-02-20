test: @(x): delay x;
t: test(delay test(delay 1));
println(t = 1);
