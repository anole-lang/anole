#include "frame.hpp"
#include "noneobject.hpp"
#include "boolobject.hpp"
#include "listobject.hpp"
#include "dictobject.hpp"
#include "funcobject.hpp"
#include "thunkobject.hpp"
#include "integerobject.hpp"
#include "builtinfuncobject.hpp"

#define OPRAND(T) (any_cast<T>(code.get_instructions()[pc].oprand))

using namespace std;

namespace ice_language
{
namespace op_handles
{
void pop_handle(Ptr<Frame> frame, Code &code, size_t &pc)
{
    frame->pop();
    ++pc;
}

void create_handle(Ptr<Frame> frame, Code &code, size_t &pc)
{
    frame->scope()->create_symbol(OPRAND(string));
    ++pc;
}

void load_handle(Ptr<Frame> frame, Code &code, size_t &pc)
{
    auto name = OPRAND(string);
    auto obj = frame->scope()->load_symbol(name);

    if (!obj)
    {
        obj = frame->scope()->create_symbol(name);
    }
    if (auto thunk = dynamic_pointer_cast<ThunkObject>(*obj))
    {
        auto new_frame = make_shared<Frame>(
            frame, thunk->scope());
        new_frame->execute_code(code, thunk->base());
    }
    else
    {
        frame->push_straight(obj);
    }
    ++pc;
}

void loadconst_handle(Ptr<Frame> frame, Code &code, size_t &pc)
{
    frame->push(code.load_const(OPRAND(size_t)));
    ++pc;
}

void loadmember_handle(Ptr<Frame> frame, Code &code, size_t &pc)
{
    auto name = OPRAND(string);
    frame->push_straight(frame->pop()->load_member(name));
    ++pc;
}

void store_handle(Ptr<Frame> frame, Code &code, size_t &pc)
{
    auto p = frame->pop_straight();
    *p = frame->pop();
    frame->push_straight(p);
    ++pc;
}

void neg_handle(Ptr<Frame> frame, Code &code, size_t &pc)
{
    frame->push(frame->pop()->neg());
    ++pc;
}

void add_handle(Ptr<Frame> frame, Code &code, size_t &pc)
{
    auto rhs = frame->pop();
    auto lhs = frame->top();
    frame->set_top(lhs->add(rhs));
    ++pc;
}

void sub_handle(Ptr<Frame> frame, Code &code, size_t &pc)
{
    auto rhs = frame->pop();
    auto lhs = frame->top();
    frame->set_top(lhs->sub(rhs));
    ++pc;
}

void mul_handle(Ptr<Frame> frame, Code &code, size_t &pc)
{
    auto rhs = frame->pop();
    auto lhs = frame->top();
    frame->set_top(lhs->mul(rhs));
    ++pc;
}

void div_handle(Ptr<Frame> frame, Code &code, size_t &pc)
{
    auto rhs = frame->pop();
    auto lhs = frame->top();
    frame->set_top(lhs->div(rhs));
    ++pc;
}
void mod_handle(Ptr<Frame> frame, Code &code, size_t &pc)
{
    auto rhs = frame->pop();
    auto lhs = frame->top();
    frame->set_top(lhs->mod(rhs));
    ++pc;
}

void ceq_handle(Ptr<Frame> frame, Code &code, size_t &pc)
{
    auto rhs = frame->pop();
    auto lhs = frame->top();
    frame->set_top(lhs->ceq(rhs));
    ++pc;
}

void cne_handle(Ptr<Frame> frame, Code &code, size_t &pc)
{
    auto rhs = frame->pop();
    auto lhs = frame->top();
    frame->set_top(lhs->cne(rhs));
    ++pc;
}

void clt_handle(Ptr<Frame> frame, Code &code, size_t &pc)
{
    auto rhs = frame->pop();
    auto lhs = frame->top();
    frame->set_top(lhs->clt(rhs));
    ++pc;
}

void cle_handle(Ptr<Frame> frame, Code &code, size_t &pc)
{
    auto rhs = frame->pop();
    auto lhs = frame->top();
    frame->set_top(lhs->cle(rhs));
    ++pc;
}

void index_handle(Ptr<Frame> frame, Code &code, size_t &pc)
{
    auto obj = frame->pop();
    auto index = frame->pop();
    frame->push_straight(obj->index(index));
    ++pc;
}

void scopebegin_handle(Ptr<Frame> frame, Code &code, size_t &pc)
{
    frame->set_scope(make_shared<Scope>(frame->scope()));
    ++pc;
}

void scopeend_handle(Ptr<Frame> frame, Code &code, size_t &pc)
{
    frame->set_scope(frame->scope()->pre());
    ++pc;
}

void call_handle(Ptr<Frame> frame, Code &code, size_t &pc)
{
    if (dynamic_pointer_cast<FunctionObject>(frame->top()))
    {
        auto func = frame->pop<FunctionObject>();
        auto new_frame = make_shared<Frame>(
            frame, func->scope());
        auto call_agrs_size = OPRAND(size_t);
        for (size_t i = call_agrs_size;
            i < func->args_size(); ++i)
        {
            new_frame->push(theNone);
        }
        for (size_t i = 0; i < call_agrs_size; ++i)
        {
            new_frame->push(frame->pop());
        }
        new_frame->execute_code(code, func->base());
    }
    else if (dynamic_pointer_cast<BuiltInFunctionObject>(frame->top()))
    {
        auto builtin = frame->pop<BuiltInFunctionObject>();
        (*builtin)(frame);
    }
    ++pc;
}

void return_handle(Ptr<Frame> frame, Code &code, size_t &pc)
{
    frame->set_return();
}

void jump_handle(Ptr<Frame> frame, Code &code, size_t &pc)
{
    pc = OPRAND(size_t);
}

void jumpif_handle(Ptr<Frame> frame, Code &code, size_t &pc)
{
    if (frame->pop()->to_bool())
    {
        pc = OPRAND(size_t);
    }
    else
    {
        ++pc;
    }
}

void jumpifnot_handle(Ptr<Frame> frame, Code &code, size_t &pc)
{
    if (!frame->pop()->to_bool())
    {
        pc = OPRAND(size_t);
    }
    else
    {
        ++pc;
    }
}

void match_handle(Ptr<Frame> frame, Code &code, size_t &pc)
{
    auto key = frame->pop();
    if (frame->top()->ceq(key)->to_bool())
    {
        frame->pop();
        pc = OPRAND(size_t);
    }
    else
    {
        ++pc;
    }
}

void lambdadecl_handle(Ptr<Frame> frame, Code &code, size_t &pc)
{
    auto args_size = OPRAND(size_t);
    frame->push(make_shared<FunctionObject>(
        frame->scope(), ++pc + 1, args_size));
    pc = OPRAND(size_t);
}

void thunkdecl_handle(Ptr<Frame> frame, Code &code, size_t &pc)
{
    frame->push(make_shared<ThunkObject>(
        frame->scope(), pc + 1));
    pc = OPRAND(size_t);
}

void buildlist_handle(Ptr<Frame> frame, Code &code, size_t &pc)
{
    auto list = make_shared<ListObject>();
    auto size = OPRAND(size_t);
    while (size--)
    {
        list->append(frame->pop());
    }
    frame->push(list);
    ++pc;
}

void builddict_handle(Ptr<Frame> frame, Code &code, size_t &pc)
{
    auto dict = make_shared<DictObject>();
    auto size = OPRAND(size_t);
    while (size--)
    {
        auto key = frame->pop();
        dict->insert(key, frame->pop());
    }
    frame->push(dict);
    ++pc;
}
}

using OpHandle = void (*)(Ptr<Frame>, Code &, std::size_t &);

constexpr OpHandle theOpHandles[] =
{
    nullptr,

    &op_handles::pop_handle,

    &op_handles::create_handle,
    &op_handles::load_handle,
    &op_handles::loadconst_handle,
    &op_handles::loadmember_handle,
    &op_handles::store_handle,

    &op_handles::neg_handle,
    &op_handles::add_handle,
    &op_handles::sub_handle,
    &op_handles::mul_handle,
    &op_handles::div_handle,
    &op_handles::mod_handle,

    &op_handles::ceq_handle,
    &op_handles::cne_handle,
    &op_handles::clt_handle,
    &op_handles::cle_handle,

    &op_handles::index_handle,

    &op_handles::scopebegin_handle,
    &op_handles::scopeend_handle,

    &op_handles::call_handle,
    &op_handles::return_handle,
    &op_handles::jump_handle,
    &op_handles::jumpif_handle,
    &op_handles::jumpifnot_handle,
    &op_handles::match_handle,
    &op_handles::lambdadecl_handle,
    &op_handles::thunkdecl_handle,

    &op_handles::buildlist_handle,
    &op_handles::builddict_handle,
};

void Frame::execute_code(Code &code, size_t base)
{
    theCurrentFrame = shared_from_this();

    auto pc = base;
    while (pc < code.size() && !has_return_)
    {
        theOpHandles[code.get_instructions()[pc].opcode]
            (shared_from_this(), code, pc);
    }

    theCurrentFrame = return_to_;
}
}
