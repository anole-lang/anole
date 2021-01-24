#include "compiler.hpp"

#include "../objects/objects.hpp"

#include <set>
#include <tuple>

namespace anole
{
void BlockExpr::codegen(Code &code)
{
    for (auto &statement : statements)
    {
        statement->codegen(code);
    }
}

void NoneExpr::codegen(Code &code)
{
    code.add_ins<Opcode::LoadConst, Size>(0);
}

void IntegerExpr::codegen(Code &code)
{
    code.add_ins<Opcode::LoadConst, Size>(
        code.create_const<IntegerObject>(
            'i' + std::to_string(value), value
        )
    );
}

void FloatExpr::codegen(Code &code)
{
    code.add_ins<Opcode::LoadConst, Size>(
        code.create_const<FloatObject>(
            'f' + std::to_string(value), value
        )
    );
}

void BoolExpr::codegen(Code &code)
{
    code.add_ins<Opcode::LoadConst, Size>(value ? 1 : 2);
}

void StringExpr::codegen(Code &code)
{
    code.add_ins<Opcode::LoadConst, Size>(
        code.create_const<StringObject>(
            's' + value, value
        )
    );
}

void IdentifierExpr::codegen(Code &code)
{
    code.locate(location);
    code.add_ins<Opcode::Load, String>(name);
}

void ParenOperatorExpr::codegen(Code &code)
{
    if (args.empty())
    {
        expr->codegen(code);
        code.locate(location);
        code.add_ins<Opcode::FastCall>();
        return;
    }

    code.add_ins<Opcode::CallAc>();
    for (auto it = args.rbegin(); it != args.rend(); ++it)
    {
        (*it).first->codegen(code);
        if ((*it).second)
        {
            code.add_ins<Opcode::Unpack>();
        }
    }
    expr->codegen(code);
    code.locate(location);
    code.add_ins<Opcode::Call>();
}

void UnaryOperatorExpr::codegen(Code &code)
{
    switch (op.type)
    {
    case TokenType::Sub:
        expr->codegen(code);
        code.locate(location);
        code.add_ins<Opcode::Neg>();
        break;

    case TokenType::Not:
    {
        expr->codegen(code);
        code.locate(location);
        auto o1 = code.add_ins();
        code.add_ins<Opcode::LoadConst, Size>(1); // BoolObject::the_true()
        auto o2 = code.add_ins();
        code.set_ins<Opcode::JumpIf, Size>(o1, code.size());
        code.add_ins<Opcode::LoadConst, Size>(2); // BoolObject::the_false()
        code.set_ins<Opcode::Jump, Size>(o2, code.size());
    }
        break;

    case TokenType::BNeg:
        expr->codegen(code);
        code.locate(location);
        code.add_ins<Opcode::BNeg>();
        break;

    default:
    {
        code.add_ins<Opcode::CallAc>();
        expr->codegen(code);
        code.add_ins<Opcode::Load, String>(op.value);
        code.add_ins<Opcode::Call>();
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
        code.locate(location);
        code.add_ins<Opcode::Add>();
        break;

    case TokenType::Sub:
        lhs->codegen(code);
        rhs->codegen(code);
        code.locate(location);
        code.add_ins<Opcode::Sub>();
        break;

    case TokenType::Mul:
        lhs->codegen(code);
        rhs->codegen(code);
        code.locate(location);
        code.add_ins<Opcode::Mul>();
        break;

    case TokenType::Div:
        lhs->codegen(code);
        rhs->codegen(code);
        code.locate(location);
        code.add_ins<Opcode::Div>();
        break;

    case TokenType::Mod:
        lhs->codegen(code);
        rhs->codegen(code);
        code.locate(location);
        code.add_ins<Opcode::Mod>();
        break;

    case TokenType::BAnd:
        lhs->codegen(code);
        rhs->codegen(code);
        code.locate(location);
        code.add_ins<Opcode::BAnd>();
        break;

    case TokenType::BOr:
        lhs->codegen(code);
        rhs->codegen(code);
        code.locate(location);
        code.add_ins<Opcode::BOr>();
        break;

    case TokenType::BXor:
        lhs->codegen(code);
        rhs->codegen(code);
        code.locate(location);
        code.add_ins<Opcode::BXor>();
        break;

    case TokenType::BLS:
        lhs->codegen(code);
        rhs->codegen(code);
        code.locate(location);
        code.add_ins<Opcode::BLS>();
        break;

    case TokenType::BRS:
        lhs->codegen(code);
        rhs->codegen(code);
        code.locate(location);
        code.add_ins<Opcode::BRS>();
        break;

    case TokenType::And:
    {
        lhs->codegen(code);
        code.locate(location);
        auto o1 = code.add_ins();
        rhs->codegen(code);
        code.locate(location);
        auto o2 = code.add_ins();
        code.add_ins<Opcode::LoadConst, Size>(1);
        auto o3 = code.add_ins();
        code.set_ins<Opcode::JumpIfNot, Size>(o1, code.size());
        code.set_ins<Opcode::JumpIfNot, Size>(o2, code.size());
        code.add_ins<Opcode::LoadConst, Size>(2);
        code.set_ins<Opcode::Jump, Size>(o3, code.size());
    }
        break;

    case TokenType::Or:
    {
        lhs->codegen(code);
        code.locate(location);
        auto o1 = code.add_ins();
        rhs->codegen(code);
        code.locate(location);
        auto o2 = code.add_ins();
        code.add_ins<Opcode::LoadConst, Size>(2);
        auto o3 = code.add_ins();
        code.set_ins<Opcode::JumpIf, Size>(o1, code.size());
        code.set_ins<Opcode::JumpIf, Size>(o2, code.size());
        code.add_ins<Opcode::LoadConst, Size>(1);
        code.set_ins<Opcode::Jump, Size>(o3, code.size());
    }
        break;

    case TokenType::Is:
        lhs->codegen(code);
        rhs->codegen(code);
        code.locate(location);
        code.add_ins<Opcode::Is>();
        break;

    case TokenType::CEQ:
        lhs->codegen(code);
        rhs->codegen(code);
        code.locate(location);
        code.add_ins<Opcode::CEQ>();
        break;

    case TokenType::CNE:
        lhs->codegen(code);
        rhs->codegen(code);
        code.locate(location);
        code.add_ins<Opcode::CNE>();
        break;

    case TokenType::CLT:
        lhs->codegen(code);
        rhs->codegen(code);
        code.locate(location);
        code.add_ins<Opcode::CLT>();
        break;

    case TokenType::CLE:
        lhs->codegen(code);
        rhs->codegen(code);
        code.locate(location);
        code.add_ins<Opcode::CLE>();
        break;

    case TokenType::CGT:
        rhs->codegen(code);
        lhs->codegen(code);
        code.locate(location);
        code.add_ins<Opcode::CLE>();
        break;

    case TokenType::CGE:
        rhs->codegen(code);
        lhs->codegen(code);
        code.locate(location);
        code.add_ins<Opcode::CLT>();
        break;

    default:
        code.add_ins<Opcode::CallAc>();
        rhs->codegen(code);
        lhs->codegen(code);
        code.add_ins<Opcode::Load, String>(op.value);
        code.locate(location);
        code.add_ins<Opcode::Call>();
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
            // copy the last instruction and then reset it to Pack
            code.add_ins(code.ins_at(code.size() - 1));
            code.set_ins<Opcode::Pack>(code.size() - 2);
        }
    }
    block->codegen(code);

    code.add_ins<Opcode::ReturnNone>();
    code.set_ins<Opcode::LambdaDecl, std::pair<Size, Size>>(o1, std::make_pair<Size, Size>(parameters.size(), code.size()));
}

void DotExpr::codegen(Code &code)
{
    left->codegen(code);
        code.locate(location);
    code.add_ins<Opcode::LoadMember, String>(name);
}

void EnumExpr::codegen(Code &code)
{
    code.add_ins<Opcode::NewScope>();
    for (auto &decl : decls)
    {
        decl.expr->codegen(code);
        code.add_ins<Opcode::StoreRef, String>(decl.name);
    }
    code.add_ins<Opcode::BuildEnum>();
}

void MatchExpr::codegen(Code &code)
{
    expr->codegen(code);

    std::map<Size, std::vector<Size>> matchfroms;
    std::vector<Size> jumpfroms;

    for (Size i = 0; i < keylists.size(); ++i)
    {
        for (auto &key : keylists[i])
        {
            key->codegen(code);
            code.locate(locations[i]);
            matchfroms[i].push_back(code.add_ins());
        }
    }

    else_expr ? else_expr->codegen(code) : NoneExpr().codegen(code);
    jumpfroms.push_back(code.add_ins());

    for (Size i = 0; i < values.size(); ++i)
    {
        for (auto where : matchfroms[i])
        {
            code.set_ins<Opcode::Match, Size>(where, code.size());
        }
        values[i]->codegen(code);
        jumpfroms.push_back(code.add_ins());
    }

    for (auto where : jumpfroms)
    {
        code.set_ins<Opcode::Jump, Size>(where, code.size());
    }
}

void ListExpr::codegen(Code &code)
{
    for (auto it = exprs.rbegin();
        it != exprs.rend(); ++it)
    {
        (*it)->codegen(code);
    }
    code.add_ins<Opcode::BuildList, Size>(exprs.size());
}

void IndexExpr::codegen(Code &code)
{
    index->codegen(code);
    expr->codegen(code);

    code.locate(location);
    code.add_ins<Opcode::Index>();
}

void DictExpr::codegen(Code &code)
{
    auto rbegin_values = values.rbegin();
    auto rbegin_keys = keys.rbegin();
    while (rbegin_values != values.rend())
    {
        (*rbegin_values)->codegen(code);
        (*rbegin_keys)->codegen(code);
        ++rbegin_values; ++rbegin_keys;
    }
    code.add_ins<Opcode::BuildDict, Size>(keys.size());
}

/**
 * loaded bases...
 * BuildClass
 * load and store members...
 * EndScope
*/
void ClassExpr::codegen(Code &code)
{
    code.add_ins<Opcode::CallAc>();
    for (auto it = bases.rbegin(); it != bases.rend(); ++it)
    {
        (*it).first->codegen(code);
        if ((*it).second)
        {
            code.add_ins<Opcode::Unpack>();
        }
    }
    code.add_ins<Opcode::BuildClass>(name);

    for (auto &member : members)
    {
        member->codegen(code);

        if (dynamic_cast<VariableDeclarationStmt *>(member.get()))
        {
            auto decl = dynamic_cast<VariableDeclarationStmt *>(member.get());
            if (decl->name == "__init__")
            {
                if (!dynamic_cast<LambdaExpr *>(decl->expr.get()))
                {
                    throw CompileError("__init__ must be with function body");
                }
            }
        }
        else
        {
            auto decls = dynamic_cast<MultiVarsDeclarationStmt *>(member.get());
            for (auto &decl : decls->decls)
            {
                if (decl.name == "__init__")
                {
                    if (!dynamic_cast<LambdaExpr *>(decl.expr.get()))
                    {
                        throw CompileError("__init__ must be with function body");
                    }
                }
            }
        }
    }

    code.add_ins<Opcode::EndScope>();
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
    code.locate(location);
    auto o1 = code.add_ins();
    true_expr->codegen(code);
    auto o2 = code.add_ins();
    code.set_ins<Opcode::JumpIfNot, Size>(o1, code.size());
    false_expr->codegen(code);
    code.set_ins<Opcode::Jump, Size>(o2, code.size());
}

void UseStmt::codegen(Code &code)
{
    auto part_import = [&code] (auto begin, auto end) mutable
    {
        for (auto it = begin; it != end; ++it)
        {
            // assume type is name
            code.add_ins<Opcode::ImportPart, String>(it->mod);
        }
    };

    auto module_import = [&code, &part_import] (auto begin, auto end) mutable
    {
        if (begin->type == Module::Type::Name)
        {
            code.add_ins<Opcode::Import, String>(begin->mod);
        }
        else
        {
            code.add_ins<Opcode::ImportPath, String>(begin->mod);
        }
        part_import(begin + 1, end);
    };

    if (from.empty())
    {
        for (auto &alias : aliases)
        {
            module_import(alias.first.begin(), alias.first.end());
            code.add_ins<Opcode::StoreRef, String>(alias.second);

            if (alias.first.size() > 1)
            {
                code.add_ins<Opcode::FastPop, Size>(alias.first.size() - 1);
            }
        }
    }
    else
    {
        // means `use *`
        if (aliases.empty())
        {
            module_import(from.begin(), from.end());
            code.add_ins<Opcode::ImportAll>();

            if (from.size() > 1)
            {
                code.add_ins<Opcode::FastPop, Size>(from.size() - 1);
            }
        }
        else
        {
            module_import(from.begin(), from.end());

            for (auto &alias : aliases)
            {
                part_import(alias.first.begin(), alias.first.end());
                code.add_ins<Opcode::StoreRef, String>(alias.second);

                if (alias.first.size() > 1)
                {
                    code.add_ins<Opcode::FastPop, Size>(alias.first.size() - 1);
                }
            }

            code.add_ins<Opcode::FastPop, Size>(from.size());
        }
    }
}

void ExprStmt::codegen(Code &code)
{
    if (dynamic_cast<ParenOperatorExpr *>(expr.get()))
    {
        code.add_ins<Opcode::CallAc>();
        expr->codegen(code);
        code.add_ins<Opcode::Pop>();
    }
    else
    {
        expr->codegen(code);
        code.add_ins<Opcode::FastPop, Size>(1);
    }
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
        code.add_ins<Opcode::StoreRef, String>(name);
    }
    else
    {
        code.add_ins<Opcode::StoreLocal, String>(name);
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
        for (Size i = 0; i < decls.size(); ++i)
        {
            std::make_unique<NoneExpr>()->codegen(code);
        }
    }

    for (auto &decl : decls)
    {
        if (decl.is_ref)
        {
            code.add_ins<Opcode::StoreRef, String>(decl.name);
        }
        else
        {
            code.add_ins<Opcode::StoreLocal, String>(decl.name);
        }
    }
}

void PrefixopDeclarationStmt::codegen(Code &code)
{
    code.add_ins<Opcode::AddPrefixOp, String>(op);
}

void InfixopDeclarationStmt::codegen(Code &code)
{
    code.add_ins<Opcode::AddInfixOp, std::pair<String, Size>>(make_pair(op, precedence));
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
    code.add_ins<Opcode::ReturnAc>();
    for (auto expr = exprs.rbegin(); expr != exprs.rend(); ++expr)
    {
        (*expr)->codegen(code);
    }
    code.add_ins<Opcode::Return>();
}

void IfElseStmt::codegen(Code &code)
{
    cond->codegen(code);
    code.locate(location);
    auto o1 = code.add_ins();
    true_block->codegen(code);
    if (false_branch)
    {
        auto o2 = code.add_ins();
        code.set_ins<Opcode::JumpIfNot, Size>(o1, code.size());
        false_branch->codegen(code);
        code.set_ins<Opcode::Jump, Size>(o2, code.size());
    }
    else
    {
        code.set_ins<Opcode::JumpIfNot, Size>(o1, code.size());
    }
}

/**
 * no block-scope
*/
void WhileStmt::codegen(Code &code)
{
    auto o1 = code.size();
    cond->codegen(code);
    code.locate(location);
    auto o2 = code.add_ins();
    block->codegen(code);
    code.add_ins<Opcode::Jump>(o1);
    code.set_ins<Opcode::JumpIfNot, Size>(o2, code.size());
    code.set_break_to(code.size(), o1);
    code.set_continue_to(o1, o1);
}

void DoWhileStmt::codegen(Code &code)
{
    auto o1 = code.size();
    block->codegen(code);
    auto o2 = code.size();
    cond->codegen(code);
    code.locate(location);
    code.add_ins<Opcode::JumpIf, Size>(o1);
    code.set_break_to(code.size(), o1);
    code.set_continue_to(o2, o1);
}

/** foreach expr as ident { stmts }
 *   is equivalent to:
 *
 *  {
 *    @& //__it_<line>: expr.__iterator__();
 *    while //__it_<line>.__has_next__() {
 *      @& ident: //__it_<line>.__next__();
 *      ... stmts ...
 *    }
 *  }
 *
 *  use // and <line> to represent the inner name and prevent duplication of names
*/
void ForeachStmt::codegen(Code &code)
{
    expr->codegen(code);

    /**
     * generate code for: @& //__it: expr.__iterator__();
    */
    code.add_ins<Opcode::LoadMember, String>("__iterator__");
    code.add_ins<Opcode::FastCall>();

    auto it_name = "//__it_" + std::to_string(code.size());
    code.add_ins<Opcode::StoreRef>(it_name);

    /**
     * generate ast for cond: //__it.__has_next__()
    */
    auto cond = std::make_unique<ParenOperatorExpr>(
        std::make_unique<DotExpr>(
            std::make_unique<IdentifierExpr>(it_name), "__has_next__"
        ),
        ArgumentList()
    );

    block->statements.push_front(nullptr);
    auto next = std::make_unique<ParenOperatorExpr>(
        std::make_unique<DotExpr>(
            std::make_unique<IdentifierExpr>(it_name), "__next__"
        ),
        ArgumentList()
    );
    if (!varname.empty())
    {
        *block->statements.begin() = std::make_unique<VariableDeclarationStmt>(
            varname, move(next), true
        );
    }
    else
    {
        *block->statements.begin() = std::make_unique<ExprStmt>(std::move(next));
    }

    WhileStmt(std::move(cond), std::move(block)).codegen(code);
}
}
