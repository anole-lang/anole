#pragma once

#include <memory>
#include <vector>

namespace ice_language
{
template <typename T>
using Ptr          = std::shared_ptr<T>;

using VoidPtr      = Ptr<void>;
using ASTPtr       = Ptr<struct AST>;
using ExprPtr      = Ptr<struct Expr>;
using StmtPtr      = Ptr<struct Stmt>;
using BlockExprPtr = Ptr<struct BlockExpr>;

using StmtList     = std::vector<Ptr<struct Stmt>>;
using ExprList     = std::vector<Ptr<struct Expr>>;
using IdentList    = std::vector<Ptr<struct IdentifierExpr>>;
using VarDeclList  = std::vector<Ptr<struct VariableDeclarationStmt>>;
}
