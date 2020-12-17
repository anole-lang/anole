#include "runtime.hpp"

#include "../error.hpp"
#include "../objects/objects.hpp"
#include "../compiler/compiler.hpp"

#include <set>
#include <stack>
#include <fstream>
#include <filesystem>

#ifdef _DEBUG
#include <iostream>
#endif

#define OPRAND(T) std::any_cast<const T &>(Context::current()->oprand())

namespace anole
{
namespace
{
std::vector<char *> lc_args;
std::map<const Variable *, String> lc_not_defineds;
}

SPtr<Context> &Context::current()
{
    static SPtr<Context> the_current = nullptr;
    return the_current;
}

void Context::set_args(int argc, char *argv[], int start)
{
    for (; start < argc; ++start)
    {
        lc_args.push_back(argv[start]);
    }
}

const std::vector<char *> &Context::get_args()
{
    return lc_args;
}

void Context::add_not_defined_symbol(const String &name, const Variable *addr)
{
    lc_not_defineds[addr] = name;
}

void Context::rm_not_defined_symbol(const Variable *addr)
{
    if (lc_not_defineds.count(addr))
    {
        lc_not_defineds.erase(addr);
    }
}

const String &Context::get_not_defined_symbol(const Variable *addr)
{
    return lc_not_defineds[addr];
}

/**
 * that each continuation continues will
 *  create a new scope pointing to the scope of the resume,
 *  so changes to variables which in the previous scope
 *  may occur when different continuations continue
*/
Context::Context(SPtr<Context> resume)
  : pre_context_(resume->pre_context_)
  , scope_(std::make_shared<Scope>(resume->scope_))
  , code_(resume->code_), pc_(resume->pc_)
  , stack_(std::make_shared<Stack>(*resume->stack_))
  , current_path_(resume->current_path_)
  , call_anchors_(resume->call_anchors_)
  , return_anchor_(resume->return_anchor_)
{
    // ...
}

Context::Context(const Context &context)
  : pre_context_(context.pre_context_)
  , scope_(context.scope_)
  , code_(context.code_), pc_(context.pc_)
  , stack_(std::make_shared<Stack>(*context.stack_))
  , current_path_(context.current_path_)
  , call_anchors_(context.call_anchors_)
  , return_anchor_(context.return_anchor_)
{
    // ...
}

Context::Context(SPtr<Code> code,
    std::filesystem::path path)
  : pre_context_(nullptr)
  , scope_(std::make_shared<Scope>(nullptr))
  , code_(code), pc_(0)
  , stack_(std::make_shared<Stack>())
  , current_path_(std::move(path))
  , return_anchor_(0)
{
    // ...
}

Context::Context(SPtr<Context> pre, SPtr<Scope> scope,
    SPtr<Code> code, Size pc)
  : pre_context_(pre)
  , scope_(std::make_shared<Scope>(scope))
  , code_(std::move(code)), pc_(pc)
  , stack_(pre->stack_)
  , current_path_(pre->current_path_)
  , return_anchor_(0)
{
    // ...
}

SPtr<Context> &Context::pre_context()
{
    return pre_context_;
}

SPtr<Scope> &Context::scope()
{
    return scope_;
}

SPtr<Code> &Context::code()
{
    return code_;
}

Size &Context::pc() noexcept
{
    return pc_;
}

const Opcode &Context::opcode() const
{
    return code_->opcode_at(pc_);
}

const std::any &Context::oprand() const
{
    return code_->oprand_at(pc_);
}

void Context::push(Object *ptr)
{
    stack_->push_back(std::make_shared<Variable>(ptr));
}

void Context::push(Address addr)
{
    stack_->push_back(addr);
}

void Context::set_top(Address addr)
{
    stack_->back() = addr;
}

void Context::set_top(Object *ptr)
{
    stack_->back() = std::make_shared<Variable>(ptr);
}

void Context::pop(Size num)
{
    for (Size i = 0; i < num; ++i)
    {
        stack_->pop_back();
    }
}

Address Context::pop_address()
{
    auto res = stack_->back();
    stack_->pop_back();
    return res;
}

Size Context::size() const
{
    return stack_->size();
}

Context::Stack *Context::get_stack()
{
    return stack_.get();
}

std::filesystem::path &Context::current_path()
{
    return current_path_;
}

void Context::set_call_anchor()
{
    call_anchors_.push(stack_->size());
}

Size Context::get_call_args_num()
{
    auto n = stack_->size() - call_anchors_.top();
    call_anchors_.pop();
    return n;
}

void Context::set_return_anchor()
{
    return_anchor_ = stack_->size();
}

Size Context::get_return_vals_num()
{
    return stack_->size() - return_anchor_;
}

namespace op_handles
{
void pop_handle()
{
    auto num = Context::current()->get_call_args_num();
    Context::current()->pop(num);
    ++Context::current()->pc();
}

void fastpop_handle()
{
    Context::current()->pop();
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

    Context::current()->push(std::move(mod));
    ++Context::current()->pc();
}

void importpath_handle()
{
    auto path = std::filesystem::path(OPRAND(String));
    if (path.is_relative())
    {
        path = Context::current()->current_path() / path;
    }
    auto mod = ModuleObject::generate(path);
    if (!mod->good())
    {
        throw RuntimeError("no such module: " + path.string());
    }

    Context::current()->push(std::move(mod));
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
    auto path = std::filesystem::path(OPRAND(String));
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
    Context::current()->push(
        Context::current()->top_ptr<ModuleObject>()->load_member(name)
    );
    ++Context::current()->pc();
}

void load_handle()
{
    const auto &name = OPRAND(String);
    auto addr = Context::current()->scope()->load_symbol(name);

    if (!*addr)
    {
        Context::add_not_defined_symbol(name, addr.get());
        Context::current()->push(addr);
        ++Context::current()->pc();
    }
    else if (!addr->ptr()->is<ObjectType::Thunk>())
    {
        Context::current()->push(addr);
        ++Context::current()->pc();
    }
    else
    {
        auto thunk = reinterpret_cast<ThunkObject *>(addr->ptr());
        if (thunk->computed())
        {
            Context::current()->push(thunk->result());
            ++Context::current()->pc();
        }
        else
        {
            Context::current()->push(addr);
            Context::current() = std::make_shared<Context>(
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
    auto address = Context::current()->pop_ptr()->load_member(name);
    if (!*address)
    {
        Context::add_not_defined_symbol(name, address.get());
    }
    Context::current()->push(address);
    ++Context::current()->pc();
}

void store_handle()
{
    auto addr = Context::current()->pop_address();
    addr->bind(Context::current()->pop_ptr());
    Context::current()->push(addr);
    Context::rm_not_defined_symbol(addr.get());
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
    Context::current()->scope()
        ->create_symbol(OPRAND(String))
            ->bind(Context::current()->pop_ptr())
    ;
    ++Context::current()->pc();
}

void newscope_handle()
{
    Context::current()->scope() = std::make_shared<Scope>(
        Context::current()->scope()
    );
    ++Context::current()->pc();
}

void endscope_handle()
{
    auto &cur_scope = Context::current()->scope();
    cur_scope = cur_scope->pre();
    ++Context::current()->pc();
}

void callac_handle()
{
    Context::current()->set_call_anchor();
    ++Context::current()->pc();
}

void call_handle()
{
    /**
     * callee should be collected
     *  before call it because variables
     *  linked to callee and the callee self
     *  may not be collected
     *
     * in particular when calling contobject
     *  variables linked to the resume context
     *  may be ignored
    */
    auto callee = Context::current()->pop_ptr();
    callee->call(Context::current()->get_call_args_num());
}

void fastcall_handle()
{
    /**
     * callee should be collected
     *  before call it because variables
     *  linked to callee and the callee self
     *  may not be collected
    */
    auto callee = Context::current()->pop_ptr();
    callee->call(0);
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

    if (n != 0 && Context::current()->get_stack() != pre_context->get_stack())
    {
        auto pre_stack = pre_context->get_stack();
        auto cur_stack = Context::current()->get_stack();

        auto begin = cur_stack->begin();
        n = cur_stack->size() - n;
        while (n--)
        {
            ++begin;
        }

        pre_stack->insert(pre_stack->end(),
            begin, cur_stack->end()
        );

        cur_stack->erase(begin, cur_stack->end());
    }
    Context::current() = pre_context;
    ++Context::current()->pc();

    Collector::try_gc();
}

void returnnone_handle()
{
    Context::current() = Context::current()->pre_context();
    Context::current()->push(NoneObject::one());
    ++Context::current()->pc();

    Collector::try_gc();
}

void jump_handle()
{
    Context::current()->pc() = OPRAND(Size);
}

void jumpif_handle()
{
    if (Context::current()->pop_ptr()->to_bool())
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
    if (!Context::current()->pop_ptr()->to_bool())
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
    auto key = Context::current()->pop_ptr();
    if (Context::current()->top_ptr()->ceq(key)->to_bool())
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
    using type = std::pair<String, Size>;
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
    if (Context::current()->top_ptr()->is<ObjectType::List>())
    {
        auto l = Context::current()->top_ptr<ListObject>();
        Context::current()->pop();
        for (auto it = l->objects().rbegin();
            it != l->objects().rend(); ++it)
        {
            Context::current()->push(*it);
        }
    }
    ++Context::current()->pc();
}

void lambdadecl_handle()
{
    using type = std::pair<Size, Size>;
    const auto &num_target = OPRAND(type);
    Context::current()->push(Allocator<Object>::alloc<FunctionObject>(
        Context::current()->scope(), Context::current()->code(),
        Context::current()->pc() + 1, num_target.first
    ));
    Context::current()->pc() = num_target.second;
}

void thunkdecl_handle()
{
    Context::current()->push(Allocator<Object>::alloc<ThunkObject>(
        Context::current()->scope(), Context::current()->code(),
        Context::current()->pc() + 1
    ));
    Context::current()->pc() = OPRAND(Size);
}

void thunkover_handle()
{
    auto result = Context::current()->pop_address();
    Context::current()->top_ptr<ThunkObject>()->set_result(result);
    Context::current()->set_top(result);
    Context::current() = Context::current()->pre_context();
    ++Context::current()->pc();
}

void neg_handle()
{
    Context::current()->push(Context::current()->pop_ptr()->neg());
    ++Context::current()->pc();
}

void add_handle()
{
    auto rhs = Context::current()->pop_ptr();
    auto lhs = Context::current()->top_ptr();
    Context::current()->set_top(lhs->add(rhs));
    ++Context::current()->pc();
}

void sub_handle()
{
    auto rhs = Context::current()->pop_ptr();
    auto lhs = Context::current()->top_ptr();
    Context::current()->set_top(lhs->sub(rhs));
    ++Context::current()->pc();
}

void mul_handle()
{
    auto rhs = Context::current()->pop_ptr();
    auto lhs = Context::current()->top_ptr();
    Context::current()->set_top(lhs->mul(rhs));
    ++Context::current()->pc();
}

void div_handle()
{
    auto rhs = Context::current()->pop_ptr();
    auto lhs = Context::current()->top_ptr();
    Context::current()->set_top(lhs->div(rhs));
    ++Context::current()->pc();
}

void mod_handle()
{
    auto rhs = Context::current()->pop_ptr();
    auto lhs = Context::current()->top_ptr();
    Context::current()->set_top(lhs->mod(rhs));
    ++Context::current()->pc();
}

void is_handle()
{
    auto rhs = Context::current()->pop_ptr();
    Context::current()->set_top(
        Context::current()->top_ptr() == rhs
        ? BoolObject::the_true()
        : BoolObject::the_false()
    );
    ++Context::current()->pc();
}

void ceq_handle()
{
    auto rhs = Context::current()->pop_ptr();
    auto lhs = Context::current()->top_ptr();
    Context::current()->set_top(lhs->ceq(rhs));
    ++Context::current()->pc();
}

void cne_handle()
{
    auto rhs = Context::current()->pop_ptr();
    auto lhs = Context::current()->top_ptr();
    Context::current()->set_top(lhs->cne(rhs));
    ++Context::current()->pc();
}

void clt_handle()
{
    auto rhs = Context::current()->pop_ptr();
    auto lhs = Context::current()->top_ptr();
    Context::current()->set_top(lhs->clt(rhs));
    ++Context::current()->pc();
}

void cle_handle()
{
    auto rhs = Context::current()->pop_ptr();
    auto lhs = Context::current()->top_ptr();
    Context::current()->set_top(lhs->cle(rhs));
    ++Context::current()->pc();
}

void bneg_handle()
{
    Context::current()->push(Context::current()->pop_ptr()->bneg());
    ++Context::current()->pc();
}

void bor_handle()
{
    auto rhs = Context::current()->pop_ptr();
    auto lhs = Context::current()->top_ptr();
    Context::current()->set_top(lhs->bor(rhs));
    ++Context::current()->pc();
}

void bxor_handle()
{
    auto rhs = Context::current()->pop_ptr();
    auto lhs = Context::current()->top_ptr();
    Context::current()->set_top(lhs->bxor(rhs));
    ++Context::current()->pc();
}

void band_handle()
{
    auto rhs = Context::current()->pop_ptr();
    auto lhs = Context::current()->top_ptr();
    Context::current()->set_top(lhs->band(rhs));
    ++Context::current()->pc();
}

void bls_handle()
{
    auto rhs = Context::current()->pop_ptr();
    auto lhs = Context::current()->top_ptr();
    Context::current()->set_top(lhs->bls(rhs));
    ++Context::current()->pc();
}

void brs_handle()
{
    auto rhs = Context::current()->pop_ptr();
    auto lhs = Context::current()->top_ptr();
    Context::current()->set_top(lhs->brs(rhs));
    ++Context::current()->pc();
}

void index_handle()
{
    auto obj = Context::current()->pop_ptr();
    auto index = Context::current()->pop_ptr();
    Context::current()->push(obj->index(index));
    ++Context::current()->pc();
}

void buildenum_handle()
{
    auto enm = Allocator<Object>::alloc<EnumObject>(
        Context::current()->scope()
    );
    Context::current()->scope() = Context::current()->scope()->pre();

    enm->scope()->pre() = nullptr;;
    Context::current()->push(enm);

    ++Context::current()->pc();
}

void buildlist_handle()
{
    auto list = Allocator<Object>::alloc<ListObject>();
    auto size = OPRAND(Size);
    while (size--)
    {
        list->append(Context::current()->pop_ptr());
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
        auto key = Context::current()->pop_ptr();
        dict->insert(key, Context::current()->pop_ptr());
    }
    Context::current()->push(dict);
    ++Context::current()->pc();
}

void buildclass_handle()
{
    auto name = OPRAND(String);
    auto cls = Allocator<Object>::alloc<ClassObject>(
        name, Context::current()->scope()
    );

    auto bctors = Allocator<Object>::alloc<ListObject>();
    auto bases_num = Context::current()->get_call_args_num();
    for (Size i = 0; i < bases_num; ++i)
    {
        auto base = Context::current()->pop_ptr();

        if (auto base_cls = dynamic_cast<ClassObject *>(base))
        {
            auto &scope = base_cls->scope();
            bool has_ctor = false;
            for (auto &sym_addr : scope->symbols())
            {
                if (sym_addr.first == "__init__")
                {
                    bctors->append(sym_addr.second->ptr());
                    has_ctor = true;
                }
                else
                {
                    cls->scope()->create_symbol(sym_addr.first, sym_addr.second->ptr());
                }
            }
            if (!has_ctor)
            {
                bctors->append(nullptr);
            }
        }
        else
        {
            throw RuntimeError("each base of one class must be one class");
        }
    }
    cls->scope()->create_symbol("bctors", bctors);

    Context::current()->push(cls);
    // declare members in the new scope of the class
    Context::current()->scope() = cls->scope();
    ++Context::current()->pc();
}
}

using OpHandle = void (*)();

constexpr OpHandle theOpHandles[] =
{
    nullptr,

    &op_handles::pop_handle,
    &op_handles::fastpop_handle,

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
    &op_handles::endscope_handle,

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
    &op_handles::buildclass_handle,
};

void Context::execute()
{
    while (Context::current()->pc() < Context::current()->code()->size())
    {
        #ifdef _DEBUG
        cerr << "run at: " << Context::current()->code()->from() << ":" << Context::current()->pc() << endl;
        #endif

        theOpHandles[static_cast<uint8_t>(Context::current()->opcode())]();
    }
}
}
