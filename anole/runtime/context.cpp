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

#define OPRAND(T) std::any_cast<const T &>(theCurrContext->oprand())

namespace fs = std::filesystem;

namespace anole
{
SPtr<fs::path> theWorkingPath = std::make_shared<fs::path>(fs::current_path());
SPtr<Context> theCurrContext = nullptr;

namespace
{
std::vector<char *> localArgs;
}

void Context::set_args(int argc, char *argv[], int start)
{
    for (; start < argc; ++start)
    {
        localArgs.push_back(argv[start]);
    }
}

const std::vector<char *> &Context::get_args()
{
    return localArgs;
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
  , call_anchors_(resume->call_anchors_)
{
    // ...
}

Context::Context(const Context &context)
  : pre_context_(context.pre_context_)
  , scope_(context.scope_)
  , code_(context.code_), pc_(context.pc_)
  , stack_(std::make_shared<Stack>(*context.stack_))
  , call_anchors_(context.call_anchors_)
{
    // ...
}

Context::Context(SPtr<Code> code)
  : pre_context_(nullptr)
  , scope_(std::make_shared<Scope>(nullptr))
  , code_(code), pc_(0)
  , stack_(std::make_shared<Stack>())
{
    // ...
}

Context::Context(SPtr<Context> pre, SPtr<Scope> scope,
    SPtr<Code> code, Size pc)
  : pre_context_(pre)
  , scope_(std::make_shared<Scope>(scope))
  , code_(std::move(code)), pc_(pc)
  , stack_(pre->stack_)
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

const Address &Context::top_address() const
{
    return stack_->back();
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

const fs::path &Context::code_path() const
{
    return code_->path();
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

namespace op_handles
{
void pop_handle()
{
    const auto &num = OPRAND(Size);
    theCurrContext->pop(num);
    ++theCurrContext->pc();
}

void import_handle()
{
    const auto &name = OPRAND(String);
    auto anole_mod = ModuleObject::generate(name);
    if (anole_mod == nullptr)
    {
        throw RuntimeError("no module named " + name);
    }

    theCurrContext->push(std::move(anole_mod));
    ++theCurrContext->pc();
}

void importpath_handle()
{
    auto path = fs::path(OPRAND(String));
    auto mod = ModuleObject::generate(path);
    if (mod == nullptr)
    {
        throw RuntimeError("no such module: " + path.string());
    }

    theCurrContext->push(std::move(mod));
    ++theCurrContext->pc();
}

void importall_handle()
{
    if (dynamic_cast<ModuleObject *>(theCurrContext->top_ptr()) == nullptr)
    {
        throw RuntimeError(theCurrContext->top_address()->called_name() + " is not a module");
    }

    auto mod = theCurrContext->pop_ptr<ModuleObject>();

    if (mod->is<ObjectType::AnoleModule>())
    {
        auto anole_mod = reinterpret_cast<AnoleModuleObject *>(mod);
        for (const auto &name_ptr : anole_mod->scope()->symbols())
        {
            theCurrContext->scope()->create_symbol(
                name_ptr.first, name_ptr.second
            );
        }
    }
    else
    {
        auto cpp_mod = reinterpret_cast<CppModuleObject *>(mod);
        auto names = cpp_mod->names();
        if (!names)
        {
            throw RuntimeError("no defined _FUNCTIONS in C++ source");
        }
        for (const auto &name : *names)
        {
            theCurrContext->scope()->create_symbol(
                name, cpp_mod->load_member(name)
            );
        }
    }

    ++theCurrContext->pc();
}

void importpart_handle()
{
    const auto &name = OPRAND(String);

    if (dynamic_cast<ModuleObject *>(theCurrContext->top_ptr()) == nullptr)
    {
        throw RuntimeError(theCurrContext->top_address()->called_name() + " is not a module");
    }

    theCurrContext->push(
        theCurrContext->top_ptr<ModuleObject>()->load_member(name)
    );
    ++theCurrContext->pc();
}

void load_handle()
{
    const auto &name = OPRAND(String);
    auto addr = theCurrContext->scope()->load_symbol(name);

    if (addr->ptr() == nullptr || !addr->ptr()->is<ObjectType::Thunk>())
    {
        theCurrContext->push(addr);
        ++theCurrContext->pc();
    }
    else
    {
        auto thunk = reinterpret_cast<ThunkObject *>(addr->ptr());
        if (thunk->computed())
        {
            theCurrContext->push(thunk->result());
            ++theCurrContext->pc();
        }
        else
        {
            theCurrContext->push(addr);
            theCurrContext = std::make_shared<Context>(
                theCurrContext, thunk->scope(), thunk->code(), thunk->base()
            );
        }
    }
}

void loadconst_handle()
{
    theCurrContext->push(theCurrContext->code()->load_const(OPRAND(Size)));
    ++theCurrContext->pc();
}

void loadmember_handle()
{
    const auto &name = OPRAND(String);
    auto address = theCurrContext->pop_ptr()->load_member(name);
    theCurrContext->push(address);
    ++theCurrContext->pc();
}

void store_handle()
{
    auto addr = theCurrContext->pop_address();
    addr->bind(theCurrContext->pop_ptr());
    theCurrContext->push(addr);
    ++theCurrContext->pc();
}

void storeref_handle()
{
    theCurrContext->scope()->create_symbol(
        OPRAND(String), theCurrContext->pop_address()
    );
    ++theCurrContext->pc();
}

void storelocal_handle()
{
    theCurrContext->scope()
        ->create_symbol(OPRAND(String))
            ->bind(theCurrContext->pop_ptr())
    ;
    ++theCurrContext->pc();
}

void newscope_handle()
{
    theCurrContext->scope() = std::make_shared<Scope>(
        theCurrContext->scope()
    );
    ++theCurrContext->pc();
}

void endscope_handle()
{
    auto &cur_scope = theCurrContext->scope();
    cur_scope = cur_scope->pre();
    ++theCurrContext->pc();
}

void callac_handle()
{
    theCurrContext->set_call_anchor();
    ++theCurrContext->pc();
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
    auto callee = theCurrContext->pop_ptr();
    callee->call(theCurrContext->get_call_args_num());
}

void fastcall_handle()
{
    /**
     * callee should be collected
     *  before call it because variables
     *  linked to callee and the callee self
     *  may not be collected
    */
    auto callee = theCurrContext->pop_ptr();
    callee->call(OPRAND(Size));
}

void return_handle()
{
    auto pre_context = theCurrContext->pre_context();
    if (theCurrContext->get_stack() != pre_context->get_stack())
    {
        pre_context->push(theCurrContext->pop_address());
    }
    theCurrContext = pre_context;
    ++theCurrContext->pc();

    Collector::try_gc();
}

void returnnone_handle()
{
    theCurrContext = theCurrContext->pre_context();
    theCurrContext->push(NoneObject::one());
    ++theCurrContext->pc();

    Collector::try_gc();
}

void jump_handle()
{
    theCurrContext->pc() = OPRAND(Size);
}

void jumpif_handle()
{
    if (theCurrContext->pop_ptr()->to_bool())
    {
        theCurrContext->pc() = OPRAND(Size);
    }
    else
    {
        ++theCurrContext->pc();
    }
}

void jumpifnot_handle()
{
    if (!theCurrContext->pop_ptr()->to_bool())
    {
        theCurrContext->pc() = OPRAND(Size);
    }
    else
    {
        ++theCurrContext->pc();
    }
}

void match_handle()
{
    auto key = theCurrContext->pop_ptr();
    if (theCurrContext->top_ptr()->ceq(key)->to_bool())
    {
        theCurrContext->pop();
        theCurrContext->pc() = OPRAND(Size);
    }
    else
    {
        ++theCurrContext->pc();
    }
}

void addprefixop_handle()
{
    Parser::add_prefixop(OPRAND(String));
    ++theCurrContext->pc();
}

void addinfixop_handle()
{
    using type = std::pair<String, Size>;
    const auto &op_p = OPRAND(type);
    Parser::add_infixop(op_p.first, op_p.second);
    ++theCurrContext->pc();
}

void pack_handle()
{
    /**
     * that pack_handle is called
     *  means no arguments now
     *
     * a empty list will be the packed result
    */
   theCurrContext->push(Allocator<Object>::alloc<ListObject>());
    ++theCurrContext->pc();
}

void unpack_handle()
{
    const auto &n = OPRAND(Size);

    if (theCurrContext->top_ptr()->is<ObjectType::List>())
    {
        auto l = theCurrContext->top_ptr<ListObject>();

        if (n && l->objects().size() != n)
        {
            throw RuntimeError(
                "expect " + std::to_string(n) +
                " but given " + std::to_string(l->objects().size())
            );
        }

        theCurrContext->pop();
        for (auto it = l->objects().rbegin();
            it != l->objects().rend(); ++it)
        {
            theCurrContext->push(*it);
        }
    }
    else
    {
        throw RuntimeError("expect list expr");
    }

    ++theCurrContext->pc();
}

void lambdadecl_handle()
{
    using type = std::pair<Size, Size>;
    const auto &num_target = OPRAND(type);
    theCurrContext->push(Allocator<Object>::alloc<FunctionObject>(
        theCurrContext->scope(), theCurrContext->code(),
        theCurrContext->pc() + 1, num_target.first
    ));
    theCurrContext->pc() = num_target.second;
}

void thunkdecl_handle()
{
    theCurrContext->push(Allocator<Object>::alloc<ThunkObject>(
        theCurrContext->scope(), theCurrContext->code(),
        theCurrContext->pc() + 1
    ));
    theCurrContext->pc() = OPRAND(Size);
}

void thunkover_handle()
{
    auto result = theCurrContext->pop_address();
    theCurrContext->top_ptr<ThunkObject>()->set_result(result);
    theCurrContext->set_top(result);
    theCurrContext = theCurrContext->pre_context();
    ++theCurrContext->pc();
}

void neg_handle()
{
    theCurrContext->push(theCurrContext->pop_ptr()->neg());
    ++theCurrContext->pc();
}

void add_handle()
{
    auto rhs = theCurrContext->pop_ptr();
    auto lhs = theCurrContext->top_ptr();
    theCurrContext->set_top(lhs->add(rhs));
    ++theCurrContext->pc();
}

void sub_handle()
{
    auto rhs = theCurrContext->pop_ptr();
    auto lhs = theCurrContext->top_ptr();
    theCurrContext->set_top(lhs->sub(rhs));
    ++theCurrContext->pc();
}

void mul_handle()
{
    auto rhs = theCurrContext->pop_ptr();
    auto lhs = theCurrContext->top_ptr();
    theCurrContext->set_top(lhs->mul(rhs));
    ++theCurrContext->pc();
}

void div_handle()
{
    auto rhs = theCurrContext->pop_ptr();
    auto lhs = theCurrContext->top_ptr();
    theCurrContext->set_top(lhs->div(rhs));
    ++theCurrContext->pc();
}

void mod_handle()
{
    auto rhs = theCurrContext->pop_ptr();
    auto lhs = theCurrContext->top_ptr();
    theCurrContext->set_top(lhs->mod(rhs));
    ++theCurrContext->pc();
}

void is_handle()
{
    auto rhs = theCurrContext->pop_ptr();
    theCurrContext->set_top(
        theCurrContext->top_ptr() == rhs
        ? BoolObject::the_true()
        : BoolObject::the_false()
    );
    ++theCurrContext->pc();
}

void ceq_handle()
{
    auto rhs = theCurrContext->pop_ptr();
    auto lhs = theCurrContext->top_ptr();
    theCurrContext->set_top(lhs->ceq(rhs));
    ++theCurrContext->pc();
}

void cne_handle()
{
    auto rhs = theCurrContext->pop_ptr();
    auto lhs = theCurrContext->top_ptr();
    theCurrContext->set_top(lhs->cne(rhs));
    ++theCurrContext->pc();
}

void clt_handle()
{
    auto rhs = theCurrContext->pop_ptr();
    auto lhs = theCurrContext->top_ptr();
    theCurrContext->set_top(lhs->clt(rhs));
    ++theCurrContext->pc();
}

void cle_handle()
{
    auto rhs = theCurrContext->pop_ptr();
    auto lhs = theCurrContext->top_ptr();
    theCurrContext->set_top(lhs->cle(rhs));
    ++theCurrContext->pc();
}

void bneg_handle()
{
    theCurrContext->push(theCurrContext->pop_ptr()->bneg());
    ++theCurrContext->pc();
}

void bor_handle()
{
    auto rhs = theCurrContext->pop_ptr();
    auto lhs = theCurrContext->top_ptr();
    theCurrContext->set_top(lhs->bor(rhs));
    ++theCurrContext->pc();
}

void bxor_handle()
{
    auto rhs = theCurrContext->pop_ptr();
    auto lhs = theCurrContext->top_ptr();
    theCurrContext->set_top(lhs->bxor(rhs));
    ++theCurrContext->pc();
}

void band_handle()
{
    auto rhs = theCurrContext->pop_ptr();
    auto lhs = theCurrContext->top_ptr();
    theCurrContext->set_top(lhs->band(rhs));
    ++theCurrContext->pc();
}

void bls_handle()
{
    auto rhs = theCurrContext->pop_ptr();
    auto lhs = theCurrContext->top_ptr();
    theCurrContext->set_top(lhs->bls(rhs));
    ++theCurrContext->pc();
}

void brs_handle()
{
    auto rhs = theCurrContext->pop_ptr();
    auto lhs = theCurrContext->top_ptr();
    theCurrContext->set_top(lhs->brs(rhs));
    ++theCurrContext->pc();
}

void index_handle()
{
    auto obj = theCurrContext->pop_ptr();
    auto index = theCurrContext->pop_ptr();
    theCurrContext->push(obj->index(index));
    ++theCurrContext->pc();
}

void buildenum_handle()
{
    auto enm = Allocator<Object>::alloc<EnumObject>(
        theCurrContext->scope()
    );
    theCurrContext->scope() = theCurrContext->scope()->pre();

    enm->scope()->pre() = nullptr;;
    theCurrContext->push(enm);

    ++theCurrContext->pc();
}

void buildlist_handle()
{
    auto list = Allocator<Object>::alloc<ListObject>();
    auto size = OPRAND(Size);
    while (size--)
    {
        list->append(theCurrContext->pop_ptr());
    }
    theCurrContext->push(list);
    ++theCurrContext->pc();
}

void builddict_handle()
{
    auto dict = Allocator<Object>::alloc<DictObject>();
    auto size = OPRAND(Size);
    while (size--)
    {
        auto key = theCurrContext->pop_ptr();
        dict->insert(key, theCurrContext->pop_ptr());
    }
    theCurrContext->push(dict);
    ++theCurrContext->pc();
}

void buildclass_handle()
{
    auto name = OPRAND(String);
    auto cls = Allocator<Object>::alloc<ClassObject>(
        name, theCurrContext->scope()
    );

    auto bctors = Allocator<Object>::alloc<ListObject>();
    auto bases_num = theCurrContext->get_call_args_num();
    for (Size i = 0; i < bases_num; ++i)
    {
        auto base = theCurrContext->pop_ptr();

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

    theCurrContext->push(cls);
    // declare members in the new scope of the class
    theCurrContext->scope() = cls->scope();
    ++theCurrContext->pc();
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
    while (theCurrContext->pc() < theCurrContext->code()->size())
    {
      #ifdef _DEBUG
        std::cerr << "run at: " << theCurrContext->code()->from() << ":" << theCurrContext->pc() << std::endl;
      #endif

        theOpHandles[static_cast<uint8_t>(theCurrContext->opcode())]();
    }
}
}
