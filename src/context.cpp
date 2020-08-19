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

#include <set>
#include <fstream>
#include <filesystem>

#ifdef _DEBUG
#include <iostream>
#endif

#define OPRAND(T) any_cast<const T &>(Context::current()->oprand())

using namespace std;

namespace anole
{
namespace
{
vector<char *> lc_args;
map<Address, String> lc_not_defineds;
stack<Size> lc_call_anchors;
stack<Size> lc_return_anchors;
}

Context *&Context::current()
{
    static Context *the_current = nullptr;
    return the_current;
}

void
Context::set_args(int argc, char *argv[], int start)
{
    for (; start < argc; ++start)
    {
        lc_args.push_back(argv[start]);
    }
}

const vector<char *>
&Context::get_args()
{
    return lc_args;
}

void
Context::add_not_defined_symbol(
    const String &name, const Address addr)
{
    lc_not_defineds[addr] = name;
}

void
Context::rm_not_defined_symbol(
    const Address addr)
{
    if (lc_not_defineds.count(addr))
    {
        lc_not_defineds.erase(addr);
    }
}

const String
&Context::get_not_defined_symbol(
    const Address addr)
{
    return lc_not_defineds[addr];
}

void Context::set_call_anchor()
{
    lc_call_anchors.push(stack_->size());
}

Size Context::get_call_args_num()
{
    auto n = stack_->size() - lc_call_anchors.top();
    lc_call_anchors.pop();
    return n;
}

void Context::set_return_anchor()
{
    lc_return_anchors.push(stack_->size());
}

Size Context::get_return_vals_num()
{
    auto n = stack_->size() - lc_return_anchors.top();
    lc_return_anchors.pop();
    return n;
}

namespace op_handles
{
void pop_handle()
{
    Context::current()->get_stack()->pop();
    ++Context::current()->pc();
}

void import_handle()
{
    const auto &name = OPRAND(String);
    auto mod = ModuleObject::generate(name);
    if (!mod->good())
    {
        throw RuntimeError("no module named " + name);
    }

    Context::current()->push(move(mod));
    ++Context::current()->pc();
}

void importpath_handle()
{
    auto path = filesystem::path(OPRAND(String));
    if (path.is_relative())
    {
        path = Context::current()->current_path() / path;
    }
    auto mod = ModuleObject::generate(path);
    if (!mod->good())
    {
        throw RuntimeError("no such module: " + path.string());
    }

    Context::current()->push(move(mod));
    ++Context::current()->pc();
}

void importall_handle()
{
    const auto &name = OPRAND(String);

    auto anole_mod = Allocator<Object>::alloc<AnoleModuleObject>(name);
    if (anole_mod->good())
    {
        for (const auto &name_ptr : anole_mod->scope()->symbols())
        {
            Context::current()->scope()->create_symbol(
                name_ptr.first, name_ptr.second
            );
        }
        ++Context::current()->pc();
        return;
    }

    auto cpp_mod = Allocator<Object>::alloc<CppModuleObject>(name);
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
        Context::current()->scope()->create_symbol(
            name, cpp_mod->load_member(name)
        );
    }
    ++Context::current()->pc();
}

void importallpath_handle()
{
    auto path = filesystem::path(OPRAND(String));
    if (path.is_relative())
    {
        path = Context::current()->current_path() / path;
    }

    auto anole_mod = Allocator<Object>::alloc<AnoleModuleObject>(path);
    if (anole_mod->good())
    {
        for (const auto &name_ptr : anole_mod->scope()->symbols())
        {
            Context::current()->scope()->create_symbol(
                name_ptr.first, name_ptr.second
            );
        }
        ++Context::current()->pc();
        return;
    }

    auto cpp_mod = Allocator<Object>::alloc<CppModuleObject>(path);
    if (!cpp_mod->good())
    {
        throw RuntimeError("no such module: " + path.string());
    }
    auto names = cpp_mod->names();
    if (!names)
    {
        throw RuntimeError("no defined _FUNCTIONS in C++ source");
    }
    for (const auto &name : *names)
    {
        Context::current()->scope()->create_symbol(
            name, cpp_mod->load_member(name)
        );
    }
    ++Context::current()->pc();
}

void importpart_handle()
{
    const auto &name = OPRAND(String);
    Context::current()->push_address(
        Context::current()->top<ModuleObject>()->load_member(name)
    );
    ++Context::current()->pc();
}

void load_handle()
{
    const auto &name = OPRAND(String);
    auto addr = Context::current()->scope()->load_symbol(name);

    if (!*addr)
    {
        Context::add_not_defined_symbol(name, addr);
        Context::current()->push_address(addr);
        ++Context::current()->pc();
    }
    else if (!addr->obj()->is<ObjectType::Thunk>())
    {
        Context::current()->push_address(addr);
        ++Context::current()->pc();
    }
    else
    {
        auto thunk = reinterpret_cast<ThunkObject *>(addr->obj());
        if (thunk->computed())
        {
            Context::current()->push_address(thunk->result());
            ++Context::current()->pc();
        }
        else
        {
            Context::current()->push_address(addr);
            Context::current() = Allocator<Context>::alloc(
                Context::current(), thunk->scope(), thunk->code(), thunk->base()
            );
        }
    }
}

void loadconst_handle()
{
    Context::current()->push(Context::current()->code()->load_const(OPRAND(Size)));
    ++Context::current()->pc();
}

void loadmember_handle()
{
    const auto &name = OPRAND(String);
    auto address = Context::current()->pop()->load_member(name);
    if (!*address)
    {
        Context::add_not_defined_symbol(name, address);
    }
    Context::current()->push_address(address);
    ++Context::current()->pc();
}

void store_handle()
{
    auto p = Context::current()->pop_address();
    *p = Context::current()->pop();
    Context::current()->push_address(p);
    Context::rm_not_defined_symbol(p);
    ++Context::current()->pc();
}

void storeref_handle()
{
    Context::current()->scope()->create_symbol(
        OPRAND(String), Context::current()->pop_address()
    );
    ++Context::current()->pc();
}

void storelocal_handle()
{
    *Context::current()->scope()
        ->create_symbol(OPRAND(String))
            = Context::current()->pop()
    ;
    ++Context::current()->pc();
}

void newscope_handle()
{
    Context::current()->scope() = Allocator<Scope>::alloc(
        Context::current()->scope()
    );
    ++Context::current()->pc();
}

void callac_handle()
{
    Context::current()->set_call_anchor();
    ++Context::current()->pc();
}

void call_handle()
{
    Context::current()->pop()->call(Context::current()->get_call_args_num());
}

void fastcall_handle()
{
    Context::current()->pop()->call(0);
}

void returnac_handle()
{
    Context::current()->set_return_anchor();
    ++Context::current()->pc();
}

void return_handle()
{
    auto n = Context::current()->get_return_vals_num();
    auto pre_context = Context::current()->pre_context();
    if (Context::current()->get_stack() != pre_context->get_stack())
    {
        Context::Stack temp;
        for (Size i = 0; i < n; ++i)
        {
            temp.push(Context::current()->pop_address());
        }
        for (Size i = 0; i < n; ++i)
        {
            pre_context->push_address(temp.top());
            temp.pop();
        }
    }
    Context::current() = pre_context;
    ++Context::current()->pc();
}

void returnnone_handle()
{
    Context::current() = Context::current()->pre_context();
    Context::current()->push(NoneObject::one());
    ++Context::current()->pc();
}

void jump_handle()
{
    Context::current()->pc() = OPRAND(Size);
}

void jumpif_handle()
{
    if (Context::current()->pop()->to_bool())
    {
        Context::current()->pc() = OPRAND(Size);
    }
    else
    {
        ++Context::current()->pc();
    }
}

void jumpifnot_handle()
{
    if (!Context::current()->pop()->to_bool())
    {
        Context::current()->pc() = OPRAND(Size);
    }
    else
    {
        ++Context::current()->pc();
    }
}

void match_handle()
{
    auto key = Context::current()->pop();
    if (Context::current()->top()->ceq(key)->to_bool())
    {
        Context::current()->pop();
        Context::current()->pc() = OPRAND(Size);
    }
    else
    {
        ++Context::current()->pc();
    }
}

void addprefixop_handle()
{
    Parser::add_prefixop(OPRAND(String));
    ++Context::current()->pc();
}

void addinfixop_handle()
{
    using type = pair<String, Size>;
    const auto &op_p = OPRAND(type);
    Parser::add_infixop(op_p.first, op_p.second);
    ++Context::current()->pc();
}

void pack_handle()
{
    /**
     * that pack_handle is called
     *  means no arguments now
     *
     * a empty list will be the packed result
    */
   Context::current()->push(Allocator<Object>::alloc<ListObject>());
    ++Context::current()->pc();
}

void unpack_handle()
{
    if (auto l = dynamic_cast<ListObject*>(Context::current()->top()))
    {
        Context::current()->pop();
        for (auto it = l->objects().rbegin();
            it != l->objects().rend(); ++it)
        {
            Context::current()->push_address(*it);
        }
    }
    ++Context::current()->pc();
}

void lambdadecl_handle()
{
    using type = pair<Size, Size>;
    const auto &num_target = OPRAND(type);
    Context::current()->push(Allocator<Object>::alloc<FunctionObject>(
        Context::current()->scope(), Context::current()->code(),
        Context::current()->pc() + 1, num_target.first)
    );
    Context::current()->pc() = num_target.second;
}

void thunkdecl_handle()
{
    Context::current()->push(Allocator<Object>::alloc<ThunkObject>(
        Context::current()->scope(), Context::current()->code(),
        Context::current()->pc() + 1)
    );
    Context::current()->pc() = OPRAND(Size);
}

void thunkover_handle()
{
    auto result = Context::current()->pop_address();
    Context::current()->top<ThunkObject>()->set_result(result);
    Context::current()->top_address() = result;
    Context::current() = Context::current()->pre_context();
    ++Context::current()->pc();
}

void neg_handle()
{
    Context::current()->push(Context::current()->pop()->neg());
    ++Context::current()->pc();
}

void add_handle()
{
    auto rhs = Context::current()->pop();
    auto lhs = Context::current()->top();
    Context::current()->set_top(lhs->add(rhs));
    ++Context::current()->pc();
}

void sub_handle()
{
    auto rhs = Context::current()->pop();
    auto lhs = Context::current()->top();
    Context::current()->set_top(lhs->sub(rhs));
    ++Context::current()->pc();
}

void mul_handle()
{
    auto rhs = Context::current()->pop();
    auto lhs = Context::current()->top();
    Context::current()->set_top(lhs->mul(rhs));
    ++Context::current()->pc();
}

void div_handle()
{
    auto rhs = Context::current()->pop();
    auto lhs = Context::current()->top();
    Context::current()->set_top(lhs->div(rhs));
    ++Context::current()->pc();
}

void mod_handle()
{
    auto rhs = Context::current()->pop();
    auto lhs = Context::current()->top();
    Context::current()->set_top(lhs->mod(rhs));
    ++Context::current()->pc();
}

void is_handle()
{
    auto rhs = Context::current()->pop();
    Context::current()->set_top(Context::current()->top() == rhs ? BoolObject::the_true() : BoolObject::the_false());
    ++Context::current()->pc();
}

void ceq_handle()
{
    auto rhs = Context::current()->pop();
    auto lhs = Context::current()->top();
    Context::current()->set_top(lhs->ceq(rhs));
    ++Context::current()->pc();
}

void cne_handle()
{
    auto rhs = Context::current()->pop();
    auto lhs = Context::current()->top();
    Context::current()->set_top(lhs->cne(rhs));
    ++Context::current()->pc();
}

void clt_handle()
{
    auto rhs = Context::current()->pop();
    auto lhs = Context::current()->top();
    Context::current()->set_top(lhs->clt(rhs));
    ++Context::current()->pc();
}

void cle_handle()
{
    auto rhs = Context::current()->pop();
    auto lhs = Context::current()->top();
    Context::current()->set_top(lhs->cle(rhs));
    ++Context::current()->pc();
}

void bneg_handle()
{
    Context::current()->push(Context::current()->pop()->bneg());
    ++Context::current()->pc();
}

void bor_handle()
{
    auto rhs = Context::current()->pop();
    auto lhs = Context::current()->top();
    Context::current()->set_top(lhs->bor(rhs));
    ++Context::current()->pc();
}

void bxor_handle()
{
    auto rhs = Context::current()->pop();
    auto lhs = Context::current()->top();
    Context::current()->set_top(lhs->bxor(rhs));
    ++Context::current()->pc();
}

void band_handle()
{
    auto rhs = Context::current()->pop();
    auto lhs = Context::current()->top();
    Context::current()->set_top(lhs->band(rhs));
    ++Context::current()->pc();
}

void bls_handle()
{
    auto rhs = Context::current()->pop();
    auto lhs = Context::current()->top();
    Context::current()->set_top(lhs->bls(rhs));
    ++Context::current()->pc();
}

void brs_handle()
{
    auto rhs = Context::current()->pop();
    auto lhs = Context::current()->top();
    Context::current()->set_top(lhs->brs(rhs));
    ++Context::current()->pc();
}

void index_handle()
{
    auto obj = Context::current()->pop();
    auto index = Context::current()->pop();
    Context::current()->push_address(obj->index(index));
    ++Context::current()->pc();
}

void buildenum_handle()
{
    Context::current()->push(Allocator<Object>::alloc<EnumObject>(
        Context::current()->scope())
    );
    Context::current()->scope() = Context::current()->scope()->pre();
    ++Context::current()->pc();
}

void buildlist_handle()
{
    auto list = Allocator<Object>::alloc<ListObject>();
    auto size = OPRAND(Size);
    while (size--)
    {
        list->append(Context::current()->pop());
    }
    Context::current()->push(list);
    ++Context::current()->pc();
}

void builddict_handle()
{
    auto dict = Allocator<Object>::alloc<DictObject>();
    auto size = OPRAND(Size);
    while (size--)
    {
        auto key = Context::current()->pop();
        dict->insert(key, Context::current()->pop());
    }
    Context::current()->push(dict);
    ++Context::current()->pc();
}
}

using OpHandle = void (*)();

constexpr OpHandle theOpHandles[] =
{
    nullptr,

    &op_handles::pop_handle,

    &op_handles::import_handle,
    &op_handles::importpath_handle,
    &op_handles::importall_handle,
    &op_handles::importallpath_handle,
    &op_handles::importpart_handle,

    &op_handles::load_handle,
    &op_handles::loadconst_handle,
    &op_handles::loadmember_handle,
    &op_handles::store_handle,
    &op_handles::storeref_handle,
    &op_handles::storelocal_handle,

    &op_handles::newscope_handle,

    &op_handles::callac_handle,
    &op_handles::call_handle,
    &op_handles::fastcall_handle,
    &op_handles::returnac_handle,
    &op_handles::return_handle,
    &op_handles::returnnone_handle,
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
    while (Context::current()->pc() < Context::current()->code()->size())
    {
        #ifdef _DEBUG
        cerr << "run at: " << Context::current()->code()->from() << ":" << Context::current()->pc() << endl;
        #endif

        theOpHandles[Context::current()->opcode()]();
    }
}
}
