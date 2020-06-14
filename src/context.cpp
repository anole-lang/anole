#ifdef _DEBUG
#include <iostream>
#endif
#include <set>
#include <fstream>
#include "error.hpp"
#include "parser.hpp"
#include "context.hpp"
#include "noneobject.hpp"
#include "boolobject.hpp"
#include "listobject.hpp"
#include "enumobject.hpp"
#include "dictobject.hpp"
#include "funcobject.hpp"
#include "contobject.hpp"
#include "thunkobject.hpp"
#include "moduleobject.hpp"
#include "integerobject.hpp"
#include "builtinfuncobject.hpp"

#define OPRAND(T) any_cast<const T &>(theCurrentContext->oprand())

using namespace std;

namespace anole
{
SPtr<Context> theCurrentContext = nullptr;

namespace
{
map<Address, string> not_defineds;
stack<size_t> call_anchors;
}

void
Context::add_not_defined_symbol(
    const string &name, const Address &ptr)
{
    not_defineds[ptr] = name;
}

void
Context::rm_not_defined_symbol(
    const Address &ptr)
{
    if (not_defineds.count(ptr))
    {
        not_defineds.erase(ptr);
    }
}

const string
&Context::get_not_defined_symbol(
    const Address &ptr)
{
    return not_defineds[ptr];
}

void Context::set_callex_anchor()
{
    call_anchors.push(stack_->size());
}

size_t Context::get_callex_args_num()
{
    auto n = stack_->size() - call_anchors.top();
    call_anchors.pop();
    return n;
}

namespace op_handles
{
void pop_handle()
{
    theCurrentContext->get_stack()->pop();
    ++theCurrentContext->pc();
}

void import_handle()
{
    const auto &name = OPRAND(string);
    auto mod = ModuleObject::generate(name);
    if (!mod->good())
    {
        throw RuntimeError("no module named " + name);
    }

    theCurrentContext->push(move(mod));
    ++theCurrentContext->pc();
}

void importpart_handle()
{
    const auto &name = OPRAND(string);
    theCurrentContext->push_address(
        theCurrentContext->top<ModuleObject>()->load_member(name));
    ++theCurrentContext->pc();
}

void importall_handle()
{
    const auto &name = OPRAND(string);

    auto anole_mod = make_shared<AnoleModuleObject>(name);
    if (anole_mod->good())
    {
        for (const auto &name_ptr : anole_mod->scope()->symbols())
        {
            theCurrentContext->scope()->create_symbol(
                name_ptr.first, name_ptr.second);
        }
        ++theCurrentContext->pc();
        return;
    }

    auto cpp_mod = make_shared<CppModuleObject>(name);
    if (!cpp_mod->good())
    {
        throw RuntimeError("no module named " + name);
    }
    auto names = cpp_mod->names();
    if (!names)
    {
        throw RuntimeError("no defined _FUNCTIONS in C++ source");
    }
    for (const auto &name : *names)
    {
        theCurrentContext->scope()->create_symbol(
            name, cpp_mod->load_member(name));
    }
    ++theCurrentContext->pc();
}

void load_handle()
{
    const auto &name = OPRAND(string);
    auto obj = theCurrentContext->scope()->load_symbol(name);

    if (!*obj)
    {
        Context::add_not_defined_symbol(name, obj);
        theCurrentContext->push_address(obj);
        ++theCurrentContext->pc();
    }
    else if ((*obj)->type() != ObjectType::Thunk)
    {
        theCurrentContext->push_address(obj);
        ++theCurrentContext->pc();
    }
    else
    {
        auto thunk = reinterpret_cast<ThunkObject *>(obj.get()->get());
        if (thunk->computed())
        {
            theCurrentContext->push_address(thunk->result());
            ++theCurrentContext->pc();
        }
        else
        {
            theCurrentContext->push_address(obj);
            theCurrentContext = make_shared<Context>(
            theCurrentContext, thunk->scope(), thunk->code(), thunk->base());
        }
    }
}

void loadconst_handle()
{
    theCurrentContext->push(theCurrentContext->code()->load_const(OPRAND(size_t)));
    ++theCurrentContext->pc();
}

void loadmember_handle()
{
    const auto &name = OPRAND(string);
    theCurrentContext
        ->push_address(
            theCurrentContext
                ->pop()
                ->load_member(name)
        );
    ++theCurrentContext->pc();
}

void store_handle()
{
    auto p = theCurrentContext->pop_address();
    *p = theCurrentContext->pop();
    theCurrentContext->push_address(p);
    Context::rm_not_defined_symbol(p);
    ++theCurrentContext->pc();
}

void storeref_handle()
{
    theCurrentContext->scope()->create_symbol(
        OPRAND(string), theCurrentContext->pop_address());
    ++theCurrentContext->pc();
}

void storelocal_handle()
{
    *theCurrentContext->scope()->create_symbol(OPRAND(string))
        = theCurrentContext->pop();
    ++theCurrentContext->pc();
}

void newscope_handle()
{
    theCurrentContext->scope() = make_shared<Scope>(theCurrentContext->scope());
    ++theCurrentContext->pc();
}

void call_handle()
{
    theCurrentContext->pop()->call(OPRAND(size_t));
}

void calltail_handle()
{
    theCurrentContext->pop()->call_tail(OPRAND(size_t));
}

void callexanchor_handle()
{
    theCurrentContext->set_callex_anchor();
    ++theCurrentContext->pc();
}

void callex_handle()
{
    theCurrentContext->pop()->call(theCurrentContext->get_callex_args_num());
}

void callextail_handle()
{

    theCurrentContext->pop()->call_tail(theCurrentContext->get_callex_args_num());
}

void return_handle()
{
    auto pre_context = theCurrentContext->pre_context();
    if (theCurrentContext->get_stack() != pre_context->get_stack())
    {
        pre_context->push_address(theCurrentContext->pop_address());
    }
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

void addprefixop_handle()
{
    Parser::add_prefixop(OPRAND(string));
    ++theCurrentContext->pc();
}

void addinfixop_handle()
{
    using type = pair<string, size_t>;
    const auto &op_p = OPRAND(type);
    Parser::add_infixop(op_p.first, op_p.second);
    ++theCurrentContext->pc();
}

void pack_handle()
{
    // if pack_handle is called, nothing will happen
    ++theCurrentContext->pc();
}

void unpack_handle()
{
    if (auto l = dynamic_cast<ListObject*>(theCurrentContext->top()))
    {
        auto handle = theCurrentContext->pop();
        for (auto it = l->objects().rbegin();
            it != l->objects().rend(); ++it)
        {
            theCurrentContext->push_address(*it);
        }
    }
    ++theCurrentContext->pc();
}

void lambdadecl_handle()
{
    using type = pair<size_t, size_t>;
    const auto &num_target = OPRAND(type);
    theCurrentContext->push(make_shared<FunctionObject>(
        theCurrentContext->scope(), theCurrentContext->code(),
        theCurrentContext->pc() + 1, num_target.first));
    theCurrentContext->pc() = num_target.second;
}

void thunkdecl_handle()
{
    theCurrentContext->push(make_shared<ThunkObject>(
        theCurrentContext->scope(), theCurrentContext->code(),
        theCurrentContext->pc() + 1));
    theCurrentContext->pc() = OPRAND(size_t);
}

void thunkover_handle()
{
    auto result = theCurrentContext->pop_address();
    theCurrentContext->top<ThunkObject>()->set_result(result);
    theCurrentContext->top_address() = result;
    theCurrentContext = theCurrentContext->pre_context();
    ++theCurrentContext->pc();
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
    theCurrentContext->set_top(theCurrentContext->top() == rhs.get() ? theTrue : theFalse);
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

void bneg_handle()
{
    theCurrentContext->push(theCurrentContext->pop()->bneg());
    ++theCurrentContext->pc();
}

void bor_handle()
{
    auto rhs = theCurrentContext->pop();
    auto lhs = theCurrentContext->top();
    theCurrentContext->set_top(lhs->bor(rhs));
    ++theCurrentContext->pc();
}

void bxor_handle()
{
    auto rhs = theCurrentContext->pop();
    auto lhs = theCurrentContext->top();
    theCurrentContext->set_top(lhs->bxor(rhs));
    ++theCurrentContext->pc();
}

void band_handle()
{
    auto rhs = theCurrentContext->pop();
    auto lhs = theCurrentContext->top();
    theCurrentContext->set_top(lhs->band(rhs));
    ++theCurrentContext->pc();
}

void bls_handle()
{
    auto rhs = theCurrentContext->pop();
    auto lhs = theCurrentContext->top();
    theCurrentContext->set_top(lhs->bls(rhs));
    ++theCurrentContext->pc();
}

void brs_handle()
{
    auto rhs = theCurrentContext->pop();
    auto lhs = theCurrentContext->top();
    theCurrentContext->set_top(lhs->brs(rhs));
    ++theCurrentContext->pc();
}

void index_handle()
{
    auto obj = theCurrentContext->pop();
    auto index = theCurrentContext->pop();
    theCurrentContext->push_address(obj->index(index));
    ++theCurrentContext->pc();
}

void buildenum_handle()
{
    theCurrentContext->push(make_shared<EnumObject>(theCurrentContext->scope()));
    theCurrentContext->scope() = theCurrentContext->scope()->pre();
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
    &op_handles::storeref_handle,
    &op_handles::storelocal_handle,

    &op_handles::newscope_handle,

    &op_handles::call_handle,
    &op_handles::calltail_handle,
    &op_handles::callexanchor_handle,
    &op_handles::callex_handle,
    &op_handles::callextail_handle,
    &op_handles::return_handle,
    &op_handles::jump_handle,
    &op_handles::jumpif_handle,
    &op_handles::jumpifnot_handle,
    &op_handles::match_handle,

    &op_handles::addprefixop_handle,
    &op_handles::addinfixop_handle,

    &op_handles::pack_handle,
    &op_handles::unpack_handle,

    &op_handles::lambdadecl_handle,
    &op_handles::thunkdecl_handle,
    &op_handles::thunkover_handle,

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

    &op_handles::bneg_handle,
    &op_handles::bor_handle,
    &op_handles::bxor_handle,
    &op_handles::band_handle,
    &op_handles::bls_handle,
    &op_handles::brs_handle,

    &op_handles::index_handle,

    &op_handles::buildenum_handle,
    &op_handles::buildlist_handle,
    &op_handles::builddict_handle,
};

void Context::execute()
{
    while (theCurrentContext->pc() < theCurrentContext->code()->size())
    {
        #ifdef _DEBUG
        cerr << "run at: " << theCurrentContext->code()->from() << ":" << theCurrentContext->pc() << endl;
        #endif

        theOpHandles[theCurrentContext->opcode()]();
    }
}
}
