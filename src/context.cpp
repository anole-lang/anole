#ifdef _DEBUG
#include <iostream>
#endif
#include <set>
#include <fstream>
#include "parser.hpp"
#include "context.hpp"
#include "noneobject.hpp"
#include "boolobject.hpp"
#include "listobject.hpp"
#include "dictobject.hpp"
#include "funcobject.hpp"
#include "contobject.hpp"
#include "thunkobject.hpp"
#include "moduleobject.hpp"
#include "integerobject.hpp"
#include "builtinfuncobject.hpp"

#define OPRAND(T) (any_cast<T>(theCurrentContext->oprand()))

using namespace std;

namespace ice_language
{
namespace op_handles
{
void pop_handle()
{
    theCurrentContext->pop();
    ++theCurrentContext->pc();
}

void import_handle()
{
    const auto name = OPRAND(string);
    ifstream fin{name + ".ice"};
    if (!fin.good())
    {
        throw runtime_error("no module named " + name);
    }
    auto code = make_shared<Code>();
    Parser(fin).gen_statements()->codegen(*code);
    auto origin = theCurrentContext;
    theCurrentContext = make_shared<Context>(code);
    Context::execute();
    origin->push(make_shared<ModuleObject>(theCurrentContext->scope()));
    theCurrentContext = origin;
    ++theCurrentContext->pc();
}

void importpart_handle()
{
    const auto name = OPRAND(string);
    theCurrentContext->push_straight(
        theCurrentContext->top<ModuleObject>()->load_member(name));
    ++theCurrentContext->pc();
}

void importall_handle()
{
    const auto name = OPRAND(string);
    ifstream fin{name + ".ice"};
    if (!fin.good())
    {
        throw runtime_error("no module named " + name);
    }
    auto code = make_shared<Code>();
    Parser(fin).gen_statements()->codegen(*code);
    auto origin = theCurrentContext;
    theCurrentContext = make_shared<Context>(code);
    Context::execute();
    auto scope = theCurrentContext->scope();
    theCurrentContext = origin;
    for (const auto &name_ptr : scope->symbols())
    {
        theCurrentContext->scope()->create_symbol(
            name_ptr.first, name_ptr.second);
    }
    ++theCurrentContext->pc();
}

void use_handle()
{
    const auto name = OPRAND(string);
    ifstream fin{name + ".ice"};
    if (!fin.good())
    {
        throw runtime_error("no module named " + name);
    }
    auto code = make_shared<Code>();
    Parser(fin).gen_statements()->codegen(*code);
    auto origin = theCurrentContext;
    theCurrentContext = make_shared<Context>(code);
    Context::execute();
    *origin->scope()->create_symbol(name)
        = make_shared<ModuleObject>(theCurrentContext->scope());
    theCurrentContext = origin;
    ++theCurrentContext->pc();
}

void load_handle()
{
    auto name = OPRAND(string);
    auto obj = theCurrentContext->scope()->load_symbol(name);

    if (!obj)
    {
        obj = theCurrentContext->scope()->create_symbol(name);
    }

    if (auto thunk = dynamic_pointer_cast<ThunkObject>(*obj))
    {
        theCurrentContext = make_shared<Context>(
            theCurrentContext, thunk->scope(), thunk->code(), thunk->base() - 1);
    }
    else
    {
        theCurrentContext->push_straight(obj);
    }
    ++theCurrentContext->pc();
}

void loadconst_handle()
{
    theCurrentContext->push(theCurrentContext->code()->load_const(OPRAND(size_t)));
    ++theCurrentContext->pc();
}

void loadmember_handle()
{
    auto name = OPRAND(string);
    theCurrentContext->push_straight(theCurrentContext->pop()->load_member(name));
    ++theCurrentContext->pc();
}

void store_handle()
{
    auto p = theCurrentContext->pop_straight();
    *p = theCurrentContext->pop();
    theCurrentContext->push_straight(p);
    ++theCurrentContext->pc();
}

void storelocal_handle()
{
    *theCurrentContext->scope()->create_symbol(OPRAND(string))
        = theCurrentContext->pop();
    ++theCurrentContext->pc();
}

void scopebegin_handle()
{
    theCurrentContext->scope() = make_shared<Scope>(theCurrentContext->scope());
    ++theCurrentContext->pc();
}

void scopeend_handle()
{
    theCurrentContext->scope() = theCurrentContext->scope()->pre();
    ++theCurrentContext->pc();
}

void call_handle()
{
    if (dynamic_pointer_cast<FunctionObject>(theCurrentContext->top()))
    {
        auto num = OPRAND(size_t);
        auto func = theCurrentContext->pop<FunctionObject>();

        theCurrentContext = make_shared<Context>(
            theCurrentContext, func->scope(), func->code(), func->base());

        auto &pc = theCurrentContext->pc();
        while (num--)
        {
            while (theCurrentContext->opcode() != Opcode::StoreLocal)
            {
                auto &ins = theCurrentContext->ins();
                if (ins.opcode == Opcode::LambdaDecl or
                    ins.opcode == Opcode::ThunkDecl)
                {
                    pc = OPRAND(size_t);
                }
                else if (++pc > theCurrentContext->code()->size())
                {
                    throw runtime_error("much arguments");
                }
            }
            *theCurrentContext->scope()->create_symbol(OPRAND(string))
                = theCurrentContext->pop();
            if (++pc > theCurrentContext->code()->size())
            {
                throw runtime_error("much arguments");
            }
        }
    }
    // builtins don't support default arguments
    else if (dynamic_pointer_cast<BuiltInFunctionObject>(theCurrentContext->top()))
    {
        theCurrentContext->pop<BuiltInFunctionObject>()->operator()();
        ++theCurrentContext->pc();
    }
    else if (dynamic_pointer_cast<ContObject>(theCurrentContext->top()))
    {
        auto resume = theCurrentContext->pop<ContObject>()->resume();
        auto retval = theCurrentContext->pop();
        theCurrentContext = make_shared<Context>(resume);
        theCurrentContext->push(retval);
        ++theCurrentContext->pc();
    }
    else
    {
        throw runtime_error("failed call with the given non-function");
    }
}

void calltail_handle()
{
    if (dynamic_pointer_cast<FunctionObject>(theCurrentContext->top()))
    {
        auto num = OPRAND(size_t);
        auto func = theCurrentContext->pop<FunctionObject>();

        theCurrentContext->scope() = make_shared<Scope>(func->scope());
        theCurrentContext->code() = func->code();
        theCurrentContext->pc() = func->base();

        auto &pc = theCurrentContext->pc();
        while (num--)
        {
            while (theCurrentContext->opcode() != Opcode::StoreLocal)
            {
                auto &ins = theCurrentContext->ins();
                if (ins.opcode == Opcode::LambdaDecl or
                    ins.opcode == Opcode::ThunkDecl)
                {
                    pc = OPRAND(size_t);
                }
                else if (++pc > theCurrentContext->code()->size())
                {
                    throw runtime_error("much arguments");
                }
            }
            *theCurrentContext->scope()->create_symbol(OPRAND(string))
                = theCurrentContext->pop();
            if (++pc > theCurrentContext->code()->size())
            {
                throw runtime_error("much arguments");
            }
        }
    }
    // builtins don't support default arguments
    else if (dynamic_pointer_cast<BuiltInFunctionObject>(theCurrentContext->top()))
    {
        theCurrentContext->pop<BuiltInFunctionObject>()->operator()();
        ++theCurrentContext->pc();
    }
    else if (dynamic_pointer_cast<ContObject>(theCurrentContext->top()))
    {
        auto resume = theCurrentContext->pop<ContObject>()->resume();
        auto retval = theCurrentContext->pop();
        theCurrentContext = make_shared<Context>(resume);
        theCurrentContext->push(retval);
        ++theCurrentContext->pc();
    }
    else
    {
        throw runtime_error("failed call with the given non-function");
    }
}

void return_handle()
{
    auto pre_context = theCurrentContext->pre_context();
    pre_context->push(theCurrentContext->pop());
    theCurrentContext = pre_context;
    ++theCurrentContext->pc();
}

void jump_handle()
{
    theCurrentContext->pc() = OPRAND(size_t);
}

void jumpif_handle()
{
    if (theCurrentContext->pop()->to_bool())
    {
        theCurrentContext->pc() = OPRAND(size_t);
    }
    else
    {
        ++theCurrentContext->pc();
    }
}

void jumpifnot_handle()
{
    if (!theCurrentContext->pop()->to_bool())
    {
        theCurrentContext->pc() = OPRAND(size_t);
    }
    else
    {
        ++theCurrentContext->pc();
    }
}

void match_handle()
{
    auto key = theCurrentContext->pop();
    if (theCurrentContext->top()->ceq(key)->to_bool())
    {
        theCurrentContext->pop();
        theCurrentContext->pc() = OPRAND(size_t);
    }
    else
    {
        ++theCurrentContext->pc();
    }
}

void lambdadecl_handle()
{
    theCurrentContext->push(make_shared<FunctionObject>(
        theCurrentContext->scope(), theCurrentContext->code(),
        theCurrentContext->pc() + 1));
    theCurrentContext->pc() = OPRAND(size_t);
}

void thunkdecl_handle()
{
    theCurrentContext->push(make_shared<ThunkObject>(
        theCurrentContext->scope(), theCurrentContext->code(),
        theCurrentContext->pc() + 1));
    theCurrentContext->pc() = OPRAND(size_t);
}

void neg_handle()
{
    theCurrentContext->push(theCurrentContext->pop()->neg());
    ++theCurrentContext->pc();
}

void add_handle()
{
    auto rhs = theCurrentContext->pop();
    auto lhs = theCurrentContext->top();
    theCurrentContext->set_top(lhs->add(rhs));
    ++theCurrentContext->pc();
}

void sub_handle()
{
    auto rhs = theCurrentContext->pop();
    auto lhs = theCurrentContext->top();
    theCurrentContext->set_top(lhs->sub(rhs));
    ++theCurrentContext->pc();
}

void mul_handle()
{
    auto rhs = theCurrentContext->pop();
    auto lhs = theCurrentContext->top();
    theCurrentContext->set_top(lhs->mul(rhs));
    ++theCurrentContext->pc();
}

void div_handle()
{
    auto rhs = theCurrentContext->pop();
    auto lhs = theCurrentContext->top();
    theCurrentContext->set_top(lhs->div(rhs));
    ++theCurrentContext->pc();
}

void mod_handle()
{
    auto rhs = theCurrentContext->pop();
    auto lhs = theCurrentContext->top();
    theCurrentContext->set_top(lhs->mod(rhs));
    ++theCurrentContext->pc();
}

void is_handle()
{
    auto rhs = theCurrentContext->pop();
    theCurrentContext->set_top(theCurrentContext->top().get() == rhs.get() ? theTrue : theFalse);
    ++theCurrentContext->pc();
}

void ceq_handle()
{
    auto rhs = theCurrentContext->pop();
    auto lhs = theCurrentContext->top();
    theCurrentContext->set_top(lhs->ceq(rhs));
    ++theCurrentContext->pc();
}

void cne_handle()
{
    auto rhs = theCurrentContext->pop();
    auto lhs = theCurrentContext->top();
    theCurrentContext->set_top(lhs->cne(rhs));
    ++theCurrentContext->pc();
}

void clt_handle()
{
    auto rhs = theCurrentContext->pop();
    auto lhs = theCurrentContext->top();
    theCurrentContext->set_top(lhs->clt(rhs));
    ++theCurrentContext->pc();
}

void cle_handle()
{
    auto rhs = theCurrentContext->pop();
    auto lhs = theCurrentContext->top();
    theCurrentContext->set_top(lhs->cle(rhs));
    ++theCurrentContext->pc();
}

void index_handle()
{
    auto obj = theCurrentContext->pop();
    auto index = theCurrentContext->pop();
    theCurrentContext->push_straight(obj->index(index));
    ++theCurrentContext->pc();
}

void buildlist_handle()
{
    auto list = make_shared<ListObject>();
    auto size = OPRAND(size_t);
    while (size--)
    {
        list->append(theCurrentContext->pop());
    }
    theCurrentContext->push(list);
    ++theCurrentContext->pc();
}

void builddict_handle()
{
    auto dict = make_shared<DictObject>();
    auto size = OPRAND(size_t);
    while (size--)
    {
        auto key = theCurrentContext->pop();
        dict->insert(key, theCurrentContext->pop());
    }
    theCurrentContext->push(dict);
    ++theCurrentContext->pc();
}
}

using OpHandle = void (*)();

constexpr OpHandle theOpHandles[] =
{
    nullptr,

    &op_handles::pop_handle,

    &op_handles::import_handle,
    &op_handles::importpart_handle,
    &op_handles::importall_handle,

    &op_handles::load_handle,
    &op_handles::loadconst_handle,
    &op_handles::loadmember_handle,
    &op_handles::store_handle,
    &op_handles::storelocal_handle,

    &op_handles::scopebegin_handle,
    &op_handles::scopeend_handle,

    &op_handles::call_handle,
    &op_handles::calltail_handle,
    &op_handles::return_handle,
    &op_handles::jump_handle,
    &op_handles::jumpif_handle,
    &op_handles::jumpifnot_handle,
    &op_handles::match_handle,

    &op_handles::lambdadecl_handle,
    &op_handles::thunkdecl_handle,

    &op_handles::neg_handle,
    &op_handles::add_handle,
    &op_handles::sub_handle,
    &op_handles::mul_handle,
    &op_handles::div_handle,
    &op_handles::mod_handle,

    &op_handles::is_handle,
    &op_handles::ceq_handle,
    &op_handles::cne_handle,
    &op_handles::clt_handle,
    &op_handles::cle_handle,

    &op_handles::index_handle,

    &op_handles::buildlist_handle,
    &op_handles::builddict_handle,
};

void Context::execute()
{
    while (theCurrentContext->pc() < theCurrentContext->code()->size())
    {
        #ifdef _DEBUG
        cout << "run at: " << theCurrentContext->pc() << endl;
        #endif

        theOpHandles[theCurrentContext->ins().opcode]();
    }
}
}
