#include <set>
#include <tuple>
#include "ast.hpp"
#include "code.hpp"
#include "parser.hpp"
#include "floatobject.hpp"
#include "stringobject.hpp"
#include "integerobject.hpp"

using namespace std;

namespace anole
{
AST::~AST() = default;
Stmt::~Stmt() = default;
Expr::~Expr() = default;

bool AST::is_integer_expr()
{
    return false;
}

bool AST::is_expr_stmt()
{
    return false;
}

bool IntegerExpr::is_integer_expr()
{
    return true;
}

bool ExprStmt::is_expr_stmt()
{
    return true;
}

void BlockExpr::codegen(Code &code)
{
    for (auto &statement : statements)
    {
        statement->codegen(code);
    }
}

void NoneExpr::codegen(Code &code)
{
    code.add_ins<Opcode::LoadConst, size_t>(0);
}

void IntegerExpr::codegen(Code &code)
{
    code.add_ins<Opcode::LoadConst>(
        code.create_const<IntegerObject>(
            'i' + to_string(value), value
        )
    );
}

void FloatExpr::codegen(Code &code)
{
    code.add_ins<Opcode::LoadConst>(
        code.create_const<FloatObject>(
            'f' + to_string(value), value
        )
    );
}

void BoolExpr::codegen(Code &code)
{
    code.add_ins<Opcode::LoadConst, size_t>(value ? 1 : 2);
}

void StringExpr::codegen(Code &code)
{
    code.add_ins<Opcode::LoadConst>(
        code.create_const<StringObject>(
            's' + value, value
        )
    );
}

void IdentifierExpr::codegen(Code &code)
{
    code.mapping(pos);
    code.add_ins<Opcode::Load>(name);
}

void ParenOperatorExpr::codegen(Code &code)
{
    bool ex = false;
    for (auto &arg : args)
    {
        ex |= arg.second;
        /**
         * for example:
         *  @foo(): 1, 2;
         *  @bar(a, b): b, a;
         *  bar(foo());
         * so extended call should be generated when there is call-expr in arguments
        */
        if (dynamic_cast<ParenOperatorExpr *>(arg.first.get()))
        {
            ex = true;
        }
    }

    if (!ex)
    {
        for (auto it = args.rbegin(); it != args.rend(); ++it)
        {
            (*it).first->codegen(code);
        }
        expr->codegen(code);
        code.mapping(pos);
        code.add_ins<Opcode::Call>(args.size());
    }
    else
    {
        code.add_ins<Opcode::CallExAnchor>();
        for (auto it = args.rbegin(); it != args.rend(); ++it)
        {
            (*it).first->codegen(code);
            if ((*it).second)
            {
                code.add_ins<Opcode::Unpack>();
            }
        }
        expr->codegen(code);
        code.mapping(pos);
        code.add_ins<Opcode::CallEx>();
    }
}

void UnaryOperatorExpr::codegen(Code &code)
{
    switch (op.type)
    {
    case TokenType::Sub:
        expr->codegen(code);
        code.mapping(pos);
        code.add_ins<Opcode::Neg>();
        break;

    case TokenType::Not:
    {
        expr->codegen(code);
        code.mapping(pos);
        auto o1 = code.add_ins();
        code.add_ins<Opcode::LoadConst, size_t>(1); // theTrue
        auto o2 = code.add_ins();
        code.set_ins<Opcode::JumpIf>(o1, code.size());
        code.add_ins<Opcode::LoadConst, size_t>(2); // theFalse
        code.set_ins<Opcode::Jump>(o2, code.size());
    }
        break;

    case TokenType::BNeg:
        expr->codegen(code);
        code.mapping(pos);
        code.add_ins<Opcode::BNeg>();
        break;

    default:
    {
        bool ex = false;
        if (dynamic_cast<ParenOperatorExpr*>(expr.get()))
        {
            ex = true;
        }

        if (ex)
        {
            code.add_ins<Opcode::CallExAnchor>();
            expr->codegen(code);
            code.add_ins<Opcode::Load>(op.value);
            code.add_ins<Opcode::CallEx>();
        }
        else
        {
            expr->codegen(code);
            code.add_ins<Opcode::Load>(op.value);
            code.add_ins<Opcode::Call>(size_t(1));
        }
    }
        break;
    }
}

void BinaryOperatorExpr::codegen(Code &code)
{
    switch (op.type)
    {
    case TokenType::Colon:
        rhs->codegen(code);
        lhs->codegen(code);
        code.add_ins<Opcode::Store>();
        break;

    case TokenType::Add:
        lhs->codegen(code);
        rhs->codegen(code);
        code.mapping(pos);
        code.add_ins<Opcode::Add>();
        break;

    case TokenType::Sub:
        lhs->codegen(code);
        rhs->codegen(code);
        code.mapping(pos);
        code.add_ins<Opcode::Sub>();
        break;

    case TokenType::Mul:
        lhs->codegen(code);
        rhs->codegen(code);
        code.mapping(pos);
        code.add_ins<Opcode::Mul>();
        break;

    case TokenType::Div:
        lhs->codegen(code);
        rhs->codegen(code);
        code.mapping(pos);
        code.add_ins<Opcode::Div>();
        break;

    case TokenType::Mod:
        lhs->codegen(code);
        rhs->codegen(code);
        code.mapping(pos);
        code.add_ins<Opcode::Mod>();
        break;

    case TokenType::BAnd:
        lhs->codegen(code);
        rhs->codegen(code);
        code.mapping(pos);
        code.add_ins<Opcode::BAnd>();
        break;

    case TokenType::BOr:
        lhs->codegen(code);
        rhs->codegen(code);
        code.mapping(pos);
        code.add_ins<Opcode::BOr>();
        break;

    case TokenType::BXor:
        lhs->codegen(code);
        rhs->codegen(code);
        code.mapping(pos);
        code.add_ins<Opcode::BXor>();
        break;

    case TokenType::BLS:
        lhs->codegen(code);
        rhs->codegen(code);
        code.mapping(pos);
        code.add_ins<Opcode::BLS>();
        break;

    case TokenType::BRS:
        lhs->codegen(code);
        rhs->codegen(code);
        code.mapping(pos);
        code.add_ins<Opcode::BRS>();
        break;

    case TokenType::And:
    {
        lhs->codegen(code);
        code.mapping(pos);
        auto o1 = code.add_ins();
        rhs->codegen(code);
        code.mapping(pos);
        auto o2 = code.add_ins();
        code.add_ins<Opcode::LoadConst, size_t>(1);
        auto o3 = code.add_ins();
        code.set_ins<Opcode::JumpIfNot>(o1, code.size());
        code.set_ins<Opcode::JumpIfNot>(o2, code.size());
        code.add_ins<Opcode::LoadConst, size_t>(2);
        code.set_ins<Opcode::Jump>(o3, code.size());
    }
        break;

    case TokenType::Or:
    {
        lhs->codegen(code);
        code.mapping(pos);
        auto o1 = code.add_ins();
        rhs->codegen(code);
        code.mapping(pos);
        auto o2 = code.add_ins();
        code.add_ins<Opcode::LoadConst, size_t>(2);
        auto o3 = code.add_ins();
        code.set_ins<Opcode::JumpIf>(o1, code.size());
        code.set_ins<Opcode::JumpIf>(o2, code.size());
        code.add_ins<Opcode::LoadConst, size_t>(1);
        code.set_ins<Opcode::Jump>(o3, code.size());
    }
        break;

    case TokenType::Is:
        lhs->codegen(code);
        rhs->codegen(code);
        code.mapping(pos);
        code.add_ins<Opcode::Is>();
        break;

    case TokenType::CEQ:
        lhs->codegen(code);
        rhs->codegen(code);
        code.mapping(pos);
        code.add_ins<Opcode::CEQ>();
        break;

    case TokenType::CNE:
        lhs->codegen(code);
        rhs->codegen(code);
        code.mapping(pos);
        code.add_ins<Opcode::CNE>();
        break;

    case TokenType::CLT:
        lhs->codegen(code);
        rhs->codegen(code);
        code.mapping(pos);
        code.add_ins<Opcode::CLT>();
        break;

    case TokenType::CLE:
        lhs->codegen(code);
        rhs->codegen(code);
        code.mapping(pos);
        code.add_ins<Opcode::CLE>();
        break;

    case TokenType::CGT:
        rhs->codegen(code);
        lhs->codegen(code);
        code.mapping(pos);
        code.add_ins<Opcode::CLE>();
        break;

    case TokenType::CGE:
        rhs->codegen(code);
        lhs->codegen(code);
        code.mapping(pos);
        code.add_ins<Opcode::CLT>();
        break;

    default:
        rhs->codegen(code);
        lhs->codegen(code);
        code.add_ins<Opcode::Load>(op.value);
        code.mapping(pos);
        code.add_ins<Opcode::Call>(size_t(2));
        break;
    }
}

void LambdaExpr::codegen(Code &code)
{
    auto o1 = code.add_ins();

    for (auto &parameter : parameters)
    {
        parameter.first->codegen(code);
        if (parameter.second)
        {
            auto store_ins = code.ins_at(code.size() - 1);
            code.set_ins<Opcode::Pack>(code.size() - 1);
            code.add_ins();
            code.ins_at(code.size() - 1) = store_ins;
        }
    }
    block->codegen(code);

    make_unique<NoneExpr>()->codegen(code);
    code.add_ins<Opcode::Return>(size_t(1));
    code.set_ins<Opcode::LambdaDecl>(o1, make_pair(parameters.size(), code.size()));
}

// [AFTER] [CLASS]
void NewExpr::codegen(Code &code)
{

}

void DotExpr::codegen(Code &code)
{
    left->codegen(code);
    code.mapping(pos);
    code.add_ins<Opcode::LoadMember>(id->name);
}

void EnumExpr::codegen(Code &code)
{
    code.add_ins<Opcode::NewScope>();
    for (auto &decl : decls)
    {
        decl->expr->codegen(code);
        code.add_ins<Opcode::StoreRef>(decl->id->name);
    }
    code.add_ins<Opcode::BuildEnum>();
}

void MatchExpr::codegen(Code &code)
{
    expr->codegen(code);

    map<size_t, vector<size_t>> matchfroms;
    vector<size_t> jumpfroms;

    for (size_t i = 0; i < keylists.size(); ++i)
    {
        for (auto &key : keylists[i])
        {
            key->codegen(code);
            code.mapping()[code.size()] = key->pos;
            matchfroms[i].push_back(code.add_ins());
        }
    }

    else_expr ? else_expr->codegen(code) : NoneExpr().codegen(code);
    jumpfroms.push_back(code.add_ins());

    for (size_t i = 0; i < values.size(); ++i)
    {
        for (auto where : matchfroms[i])
        {
            code.set_ins<Opcode::Match>(where, code.size());
        }
        values[i]->codegen(code);
        jumpfroms.push_back(code.add_ins());
    }

    for (auto where : jumpfroms)
    {
        code.set_ins<Opcode::Jump>(where, code.size());
    }
}

void ListExpr::codegen(Code &code)
{
    for (auto it = exprs.rbegin();
        it != exprs.rend(); ++it)
    {
        (*it)->codegen(code);
    }
    code.add_ins<Opcode::BuildList>(exprs.size());
}

void IndexExpr::codegen(Code &code)
{
    index->codegen(code);
    expr->codegen(code);

    code.mapping(pos);
    code.add_ins<Opcode::Index>();
}

void DictExpr::codegen(Code &code)
{
    auto num = keys.size();
    while (num--)
    {
        values[num]->codegen(code);
        keys[num]->codegen(code);
    }
    code.add_ins<Opcode::BuildDict>(keys.size());
}

void DelayExpr::codegen(Code &code)
{
    auto o1 = code.add_ins();
    expr->codegen(code);
    code.add_ins<Opcode::ThunkOver>();
    code.set_ins<Opcode::ThunkDecl>(o1, code.size());
}

void QuesExpr::codegen(Code &code)
{
    cond->codegen(code);
    code.mapping()[code.size()] = cond->pos;
    auto o1 = code.add_ins();
    true_expr->codegen(code);
    auto o2 = code.add_ins();
    code.set_ins<Opcode::JumpIfNot>(o1, code.size());
    false_expr->codegen(code);
    code.set_ins<Opcode::Jump>(o2, code.size());
}

void UseStmt::codegen(Code &code)
{
    if (from.empty())
    {
        for (auto &name_as : names)
        {
            code.add_ins<Opcode::Import>(name_as.first);
            code.add_ins<Opcode::StoreRef>(name_as.second);
        }
    }
    else
    {
        if (names.empty())
        {
            code.add_ins<Opcode::ImportAll>(from);
        }
        else
        {
            code.add_ins<Opcode::Import>(from);
            for (auto &name_as : names)
            {
                code.add_ins<Opcode::ImportPart>(name_as.first);
                code.add_ins<Opcode::StoreRef>(name_as.second);
            }
            code.add_ins<Opcode::Pop>();
        }
    }
}

void ExprStmt::codegen(Code &code)
{
    expr->codegen(code);
    code.add_ins<Opcode::Pop>();
}

void VariableDeclarationStmt::codegen(Code &code)
{
    if (expr)
    {
        expr->codegen(code);
    }
    /**
     * if expr is nullptr
     *  means that it is the parameter without default value
     *  so that we can check the number of arguments
    */

    if (is_ref)
    {
        code.add_ins<Opcode::StoreRef>(id->name);
    }
    else
    {
        code.add_ins<Opcode::StoreLocal>(id->name);
    }
}

void MultiVarsDeclarationStmt::codegen(Code &code)
{
    if (!exprs.empty())
    {
        for (auto expr = exprs.rbegin(); expr != exprs.rend(); ++expr)
        {
            (*expr)->codegen(code);
        }
    }
    else
    {
        for (size_t i = 0; i < vars.size(); ++i)
        {
            make_unique<NoneExpr>()->codegen(code);
        }
    }

    for (auto &var : vars)
    {
        if (var.second)
        {
            code.add_ins<Opcode::StoreRef>(var.first->name);
        }
        else
        {
            code.add_ins<Opcode::StoreLocal>(var.first->name);
        }
    }
}

void FunctionDeclarationStmt::codegen(Code &code)
{
    lambda->codegen(code);
    code.add_ins<Opcode::StoreRef>(id->name);
}

void PrefixopDeclarationStmt::codegen(Code &code)
{
    code.add_ins<Opcode::AddPrefixOp>(id->name);
}

void InfixopDeclarationStmt::codegen(Code &code)
{
    code.add_ins<Opcode::AddInfixOp>(make_pair(id->name, priority));
}

void ClassDeclarationStmt::codegen(Code &code)
{

}

void BreakStmt::codegen(Code &code)
{
    code.push_break(code.add_ins());
}

void ContinueStmt::codegen(Code &code)
{
    code.push_continue(code.add_ins());
}

void ReturnStmt::codegen(Code &code)
{
    for (auto expr = exprs.rbegin(); expr != exprs.rend(); ++expr)
    {
        (*expr)->codegen(code);
    }

    code.add_ins<Opcode::Return>(exprs.size());

    auto &opcode = code.opcode_at(code.size() - 2);
    if (opcode == Opcode::Call)
    {
        opcode = Opcode::CallTail;
    }
    else if (opcode == Opcode::CallEx)
    {
        opcode = Opcode::CallExTail;
    }

    /**
     * Instruction Return cannot be deleted
     * because if return cond ? true_expr, false_expr,
     * it will not return if false_expr is a tail call
     * while true_expr is not
    */
}

void IfElseStmt::codegen(Code &code)
{
    cond->codegen(code);
    code.mapping()[code.size()] = cond->pos;
    auto o1 = code.add_ins();
    true_block->codegen(code);
    if (false_branch)
    {
        auto o2 = code.add_ins();
        code.set_ins<Opcode::JumpIfNot>(o1, code.size());
        false_branch->codegen(code);
        code.set_ins<Opcode::Jump>(o2, code.size());
    }
    else
    {
        code.set_ins<Opcode::JumpIfNot>(o1, code.size());
    }
}

void WhileStmt::codegen(Code &code)
{
    auto o1 = code.size();
    cond->codegen(code);
    code.mapping()[code.size()] = cond->pos;
    auto o2 = code.add_ins();
    block->codegen(code);
    code.add_ins<Opcode::Jump>(o1);
    code.set_ins<Opcode::JumpIfNot>(o2, code.size());
    code.set_break_to(code.size(), o1);
    code.set_continue_to(o1, o1);
}

void DoWhileStmt::codegen(Code &code)
{
    auto o1 = code.size();
    block->codegen(code);
    auto o2 = code.size();
    cond->codegen(code);
    code.mapping()[code.size()] = cond->pos;
    code.add_ins<Opcode::JumpIf>(o1);
    code.set_break_to(code.size(), o1);
    code.set_continue_to(o2, o1);
}

/** foreach expr as ident { stmts }
 *    is equivalent to:
 *
 *    @&__it: expr.__iterator__();
 *    while __it.__has_next__() {
 *      @&ident: __it.__next__();
 *      ... stmts ...
 *    }
*/
void ForeachStmt::codegen(Code &code)
{
    expr->codegen(code);
    code.add_ins<Opcode::LoadMember>("__iterator__"s);
    code.add_ins<Opcode::Call>(size_t(0));
    code.add_ins<Opcode::StoreRef>("__it"s);

    auto cond = make_unique<ParenOperatorExpr>(
        make_unique<DotExpr>(make_unique<IdentifierExpr>("__it"),
            make_unique<IdentifierExpr>("__has_next__")),
        ArgumentList());

    block->statements.insert(block->statements.begin(), nullptr);
    auto next = make_unique<ParenOperatorExpr>(
        make_unique<DotExpr>(make_unique<IdentifierExpr>("__it"),
            make_unique<IdentifierExpr>("__next__")),
        ArgumentList());
    if (id != nullptr)
    {
        *block->statements.begin()
            = make_unique<VariableDeclarationStmt>(move(id), move(next), true);
    }
    else
    {
        *block->statements.begin() = make_unique<ExprStmt>(move(next));
    }

    WhileStmt(move(cond), move(block)).codegen(code);
}
}
