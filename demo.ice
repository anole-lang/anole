@@Math()
{
    @add(a, b): a + b

    @mul: @(a, b){
        return a * b
    }

    @quadraticSum: @(a, b){
        @sqrt: @(n){ return n * n }
        return @(a, b){ return a + b }(sqrt(a), sqrt(b))
    }
}