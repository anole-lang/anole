#include "../src/parser.hpp"
#include "tester.hpp"

using namespace std;
using namespace ice_language;

template <typename R, typename ...T>
shared_ptr<R> cast(shared_ptr<AST> node)
{
    ASSERT(node);
    if constexpr (sizeof...(T) > 0)
    {
        node = cast<T...>(node);
    }
    auto r = dynamic_pointer_cast<R>(node);
    ASSERT(r);
    return r;
}

TEST_CLASS(ParseTerm)
    TEST_METHOD(ParseIdentExpr)
        istringstream ss(R"(identifier)");
        auto identExpr = cast<IdentifierExpr>(cast<ExprStmt>(Parser(ss).gen_statement())->expr);
        ASSERT(identExpr->name == "identifier");
    TEST_END

    TEST_METHOD(ParseNumericExpr)
        istringstream ss(R"(1 2.3)");
        Parser parser{ss};
        auto integerExpr = cast<IntegerExpr>(cast<ExprStmt>(parser.gen_statement())->expr);
        auto floatExpr = cast<FloatExpr>(cast<ExprStmt>(parser.gen_statement())->expr);
        ASSERT(integerExpr->value == 1);
        ASSERT(floatExpr->value == 2.3);
    TEST_END

    TEST_METHOD(ParseNoneExpr)
        istringstream ss(R"(none)");
        auto noneExpr = cast<NoneExpr>(cast<ExprStmt>(Parser(ss).gen_statement())->expr);
    TEST_END

    TEST_METHOD(ParseBoolExpr)
        istringstream ss(R"(true false)");
        Parser parser{ss};
        auto trueExpr = cast<BoolExpr>(cast<ExprStmt>(parser.gen_statement())->expr);
        auto falseExpr = cast<BoolExpr>(cast<ExprStmt>(parser.gen_statement())->expr);
        ASSERT(trueExpr->value == true);
        ASSERT(falseExpr->value == false);
    TEST_END

    TEST_METHOD(ParseStringExpr)
        istringstream ss(R"("Do you love me? Ouch! Of course! \a")");
        auto strExpr = cast<StringExpr>(cast<ExprStmt>(Parser(ss).gen_statement())->expr);
        ASSERT(strExpr->value == "Do you love me? Ouch! Of course! \a");
    TEST_END

    TEST_METHOD(ParseEmptyLambdaExpr)
        istringstream ss(R"(@(){})");
        auto lambdaExpr = cast<LambdaExpr>(cast<ExprStmt>(Parser(ss).gen_statement())->expr);
        ASSERT(lambdaExpr->arg_decls.empty());
        ASSERT(lambdaExpr->block->statements.empty());
    TEST_END

    TEST_METHOD(ParseComplexLambdaExpr)
        istringstream ss(R"(@(): @(): @(){})");
        auto lambdaExpr = cast<LambdaExpr>(cast<ExprStmt>(Parser(ss).gen_statement())->expr);
        lambdaExpr = cast<LambdaExpr>(cast<ReturnStmt>(lambdaExpr->block->statements[0])->expr);
        lambdaExpr = cast<LambdaExpr>(cast<ReturnStmt>(lambdaExpr->block->statements[0])->expr);
        ASSERT(lambdaExpr->arg_decls.empty());
        ASSERT(lambdaExpr->block->statements.empty());
    TEST_END

    TEST_METHOD(ParseNewExpr)
        istringstream ss(R"(new A(1, 2.3))");
        auto newExpr = cast<NewExpr>(cast<ExprStmt>(Parser(ss).gen_statement())->expr);
        ASSERT(newExpr->id->name == "A");
        ASSERT(newExpr->args.size() == 2);
        ASSERT(cast<IntegerExpr>(newExpr->args[0])->value == 1);
        ASSERT(cast<FloatExpr>(newExpr->args[1])->value == 2.3);
    TEST_END

    TEST_METHOD(ParseListExpr)
        istringstream ss(R"([1, 2.3, none, true,
            false, "Do you love me", @(){}, new A()])");
        auto listExpr = cast<ListExpr>(cast<ExprStmt>(Parser(ss).gen_statement())->expr);
        ASSERT(listExpr->exprs.size() == 8);
        cast<IntegerExpr>(listExpr->exprs[0]);
        cast<FloatExpr>(listExpr->exprs[1]);
        cast<NoneExpr>(listExpr->exprs[2]);
        cast<BoolExpr>(listExpr->exprs[3]);
        cast<BoolExpr>(listExpr->exprs[4]);
        cast<StringExpr>(listExpr->exprs[5]);
        cast<LambdaExpr>(listExpr->exprs[6]);
        cast<NewExpr>(listExpr->exprs[7]);
    TEST_END

    TEST_METHOD(ParseEnumExpr)
        istringstream ss(R"({ ONE } { ONE, TWO })");
        Parser parser{ss};
        auto enumExpr = cast<EnumExpr>(cast<ExprStmt>(parser.gen_statement())->expr);
        ASSERT(enumExpr->idents.size() == 1);
        ASSERT(enumExpr->idents[0]->name == "ONE");
        enumExpr = cast<EnumExpr>(cast<ExprStmt>(parser.gen_statement())->expr);
        ASSERT(enumExpr->idents.size() == 2);
        ASSERT(enumExpr->idents[0]->name == "ONE");
        ASSERT(enumExpr->idents[1]->name == "TWO");
    TEST_END

    TEST_METHOD(ParseDictExpr)
        istringstream ss(R"({} {1 => 2} {3 => 4, 5 => 6})");
        Parser parser{ss};
        auto dictExpr = cast<DictExpr>(cast<ExprStmt>(parser.gen_statement())->expr);
        ASSERT(dictExpr->keys.empty() and dictExpr->values.empty());
        dictExpr = cast<DictExpr>(cast<ExprStmt>(parser.gen_statement())->expr);
        ASSERT(dictExpr->keys.size() == 1 and dictExpr->values.size() == 1);
        dictExpr = cast<DictExpr>(cast<ExprStmt>(parser.gen_statement())->expr);
        ASSERT(dictExpr->keys.size() == 2 and dictExpr->values.size() == 2);
    TEST_END
TEST_END
