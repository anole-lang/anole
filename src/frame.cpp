#ifdef _DEBUG
#include <iostream>
#endif
#include "frame.hpp"
#include "noneobject.hpp"
#include "boolobject.hpp"
#include "listobject.hpp"
#include "dictobject.hpp"
#include "funcobject.hpp"
#include "contobject.hpp"
#include "thunkobject.hpp"
#include "integerobject.hpp"
#include "builtinfuncobject.hpp"

#define OPRAND(T) (any_cast<T>(theCurrentFrame->code()->get_instructions()[theCurrentFrame->pc()].oprand))

using namespace std;

namespace ice_language
{
namespace op_handles
{
void pop_handle()
{
    theCurrentFrame->pop();
}

void create_handle()
{
    theCurrentFrame->scope()->create_symbol(OPRAND(string));
}

void load_handle()
{
    auto name = OPRAND(string);
    auto obj = theCurrentFrame->scope()->load_symbol(name);

    if (!obj)
    {
        obj = theCurrentFrame->scope()->create_symbol(name);
    }
    if (auto thunk = dynamic_pointer_cast<ThunkObject>(*obj))
    {
        theCurrentFrame->push_straight(obj);
        theCurrentFrame = make_shared<Frame>(
            theCurrentFrame, thunk->scope(), thunk->code(), thunk->base() - 1);
    }
    else
    {
        theCurrentFrame->push_straight(obj);
    }
}

void loadconst_handle()
{
    theCurrentFrame->push(theCurrentFrame->code()->load_const(OPRAND(size_t)));
}

void loadmember_handle()
{
    auto name = OPRAND(string);
    theCurrentFrame->push_straight(theCurrentFrame->pop()->load_member(name));
}

void store_handle()
{
    auto p = theCurrentFrame->pop_straight();
    *p = theCurrentFrame->pop();
    theCurrentFrame->push_straight(p);
}

void neg_handle()
{
    theCurrentFrame->push(theCurrentFrame->pop()->neg());
}

void add_handle()
{
    auto rhs = theCurrentFrame->pop();
    auto lhs = theCurrentFrame->top();
    theCurrentFrame->set_top(lhs->add(rhs));
}

void sub_handle()
{
    auto rhs = theCurrentFrame->pop();
    auto lhs = theCurrentFrame->top();
    theCurrentFrame->set_top(lhs->sub(rhs));
}

void mul_handle()
{
    auto rhs = theCurrentFrame->pop();
    auto lhs = theCurrentFrame->top();
    theCurrentFrame->set_top(lhs->mul(rhs));
}

void div_handle()
{
    auto rhs = theCurrentFrame->pop();
    auto lhs = theCurrentFrame->top();
    theCurrentFrame->set_top(lhs->div(rhs));
}

void mod_handle()
{
    auto rhs = theCurrentFrame->pop();
    auto lhs = theCurrentFrame->top();
    theCurrentFrame->set_top(lhs->mod(rhs));
}

void is_handle()
{
    auto rhs = theCurrentFrame->pop();
    theCurrentFrame->set_top(theCurrentFrame->top().get() == rhs.get() ? theTrue : theFalse);
}

void ceq_handle()
{
    auto rhs = theCurrentFrame->pop();
    auto lhs = theCurrentFrame->top();
    theCurrentFrame->set_top(lhs->ceq(rhs));
}

void cne_handle()
{
    auto rhs = theCurrentFrame->pop();
    auto lhs = theCurrentFrame->top();
    theCurrentFrame->set_top(lhs->cne(rhs));
}

void clt_handle()
{
    auto rhs = theCurrentFrame->pop();
    auto lhs = theCurrentFrame->top();
    theCurrentFrame->set_top(lhs->clt(rhs));
}

void cle_handle()
{
    auto rhs = theCurrentFrame->pop();
    auto lhs = theCurrentFrame->top();
    theCurrentFrame->set_top(lhs->cle(rhs));
}

void index_handle()
{
    auto obj = theCurrentFrame->pop();
    auto index = theCurrentFrame->pop();
    theCurrentFrame->push_straight(obj->index(index));
}

void scopebegin_handle()
{
    theCurrentFrame->scope() = make_shared<Scope>(theCurrentFrame->scope());
}

void scopeend_handle()
{
    theCurrentFrame->scope() = theCurrentFrame->scope()->pre();
}

void call_handle()
{
    if (dynamic_pointer_cast<FunctionObject>(theCurrentFrame->top()))
    {
        auto func = theCurrentFrame->pop<FunctionObject>();
        auto call_agrs_size = OPRAND(size_t);
        auto pre_frame = theCurrentFrame;
        theCurrentFrame = make_shared<Frame>(
            theCurrentFrame, func->scope(), func->code(), func->base() - 1);
        for (size_t i = call_agrs_size;
            i < func->args_size(); ++i)
        {
            theCurrentFrame->push(theNone);
        }
        for (size_t i = 0; i < call_agrs_size; ++i)
        {
            theCurrentFrame->push(pre_frame->pop());
        }
        pre_frame->push(nullptr);
    }
    else if (dynamic_pointer_cast<BuiltInFunctionObject>(theCurrentFrame->top()))
    {
        theCurrentFrame->pop<BuiltInFunctionObject>()->operator()();
    }
    else if (dynamic_pointer_cast<ContObject>(theCurrentFrame->top()))
    {
        auto resume_to = theCurrentFrame->pop<ContObject>()->resume_to();
        resume_to->push(theCurrentFrame->pop());
        theCurrentFrame = resume_to;
    }
    else
    {
        throw runtime_error("failed call with the given non-function");
    }
}

void calltail_handle()
{
    if (dynamic_pointer_cast<FunctionObject>(theCurrentFrame->top()))
    {
        auto func = theCurrentFrame->pop<FunctionObject>();

        stack<Ptr<ObjectPtr>> temp;
        for (size_t i = 0; i < OPRAND(size_t); ++i)
        {
            temp.push(theCurrentFrame->pop_straight());
        }

        theCurrentFrame->stack().swap(temp);
        theCurrentFrame->scope() = make_shared<Scope>(func->scope());
        theCurrentFrame->code() = func->code();
        theCurrentFrame->pc() = func->base() - 1;
    }
    else if (dynamic_pointer_cast<BuiltInFunctionObject>(theCurrentFrame->top()))
    {
        theCurrentFrame->pop<BuiltInFunctionObject>()->operator()();
    }
    else if (dynamic_pointer_cast<ContObject>(theCurrentFrame->top()))
    {
        auto resume_to = theCurrentFrame->pop<ContObject>()->resume_to();
        resume_to->push(theCurrentFrame->pop());
        theCurrentFrame = resume_to;
    }
    else
    {
        throw runtime_error("failed call with the given non-function");
    }
}

void return_handle()
{
    *theCurrentFrame->return_to()->top_straight() = theCurrentFrame->pop();
    theCurrentFrame = theCurrentFrame->return_to();
}

void jump_handle()
{
    theCurrentFrame->pc() = OPRAND(size_t) - 1;
}

void jumpif_handle()
{
    if (theCurrentFrame->pop()->to_bool())
    {
        theCurrentFrame->pc() = OPRAND(size_t) - 1;
    }
}

void jumpifnot_handle()
{
    if (!theCurrentFrame->pop()->to_bool())
    {
        theCurrentFrame->pc() = OPRAND(size_t) - 1;
    }
}

void match_handle()
{
    auto key = theCurrentFrame->pop();
    if (theCurrentFrame->top()->ceq(key)->to_bool())
    {
        theCurrentFrame->pop();
        theCurrentFrame->pc() = OPRAND(size_t) - 1;
    }
}

void lambdadecl_handle()
{
    auto args_size = OPRAND(size_t);
    theCurrentFrame->push(make_shared<FunctionObject>(
        theCurrentFrame->scope(), theCurrentFrame->code(), ++theCurrentFrame->pc() + 1, args_size));
    theCurrentFrame->pc() = OPRAND(size_t) - 1;
}

void thunkdecl_handle()
{
    theCurrentFrame->push(make_shared<ThunkObject>(
        theCurrentFrame->scope(), theCurrentFrame->code(), theCurrentFrame->pc() + 1));
    theCurrentFrame->pc() = OPRAND(size_t) - 1;
}

void buildlist_handle()
{
    auto list = make_shared<ListObject>();
    auto size = OPRAND(size_t);
    while (size--)
    {
        list->append(theCurrentFrame->pop());
    }
    theCurrentFrame->push(list);
}

void builddict_handle()
{
    auto dict = make_shared<DictObject>();
    auto size = OPRAND(size_t);
    while (size--)
    {
        auto key = theCurrentFrame->pop();
        dict->insert(key, theCurrentFrame->pop());
    }
    theCurrentFrame->push(dict);
}
}

using OpHandle = void (*)();

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

    &op_handles::is_handle,
    &op_handles::ceq_handle,
    &op_handles::cne_handle,
    &op_handles::clt_handle,
    &op_handles::cle_handle,

    &op_handles::index_handle,

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

    &op_handles::buildlist_handle,
    &op_handles::builddict_handle,
};

void Frame::execute()
{
    while (theCurrentFrame->pc() < theCurrentFrame->code()->size())
    {
        #ifdef _DEBUG
        cout << theCurrentFrame->pc() << endl;
        #endif

        theOpHandles[theCurrentFrame->code()->get_instructions()[theCurrentFrame->pc()].opcode]();
        ++theCurrentFrame->pc();
    }
}
}
