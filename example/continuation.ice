test: @(): call_with_current_continuation(@(cont): cont(5));
println(test());
