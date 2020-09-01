#include "runtime.hpp"

#include "../error.hpp"
#include "../objects/objects.hpp"
#include "../compiler/compiler.hpp"

#include <ctime>
#include <sstream>
#include <iostream>

using namespace std;

namespace anole
{
REGISTER_BUILTIN(eval,
{
    istringstream ss
    {
      "return " +
        dynamic_cast<StringObject *>(Context::current()->pop_rptr())->value() +
      ";"
    };

    auto code = make_shared<Code>("<eval>");
    Parser(ss, "<eval>").gen_statement()->codegen(*code);
    Context::current() = make_shared<Context>(
        Context::current(), Context::current()->scope(), code, -1
    );
    Context::current()->scope() = Context::current()->scope()->pre();
});

REGISTER_BUILTIN(call_with_current_continuation,
{
    if (Context::current()->top_rptr()->is<ObjectType::Func>())
    {
        auto handle = Context::current()->pop_sptr();
        auto func = reinterpret_cast<FunctionObject *>(handle.get());
        // copy current context
        auto cont_obj = make_shared<ContObject>(Context::current());
        Context::current() = make_shared<Context>(
            Context::current(), func->scope(), func->code(), func->base()
        );
        // the base => StoreRef/StoreLocal
        Context::current()->scope()
            ->create_symbol(any_cast<String>(
                Context::current()->oprand()))
                    ->bind(cont_obj)
        ;
    }
    else if (Context::current()->top_rptr()->is<ObjectType::Cont>())
    {
        auto resume = Context::current()->pop_rptr<ContObject>()->resume();
        auto cont_obj = make_shared<ContObject>(Context::current());
        Context::current() = make_shared<Context>(resume);
        Context::current()->push(cont_obj);
    }
    else
    {
        throw RuntimeError("err type as the argument for call/cc");
    }
});

REGISTER_BUILTIN(id,
{
    Context::current()->push(
        make_shared<IntegerObject>(
            reinterpret_cast<int64_t>(
                Context::current()->pop_rptr()))
    );
});

REGISTER_BUILTIN(print,
{
    if (Context::current()->top_rptr() != NoneObject::one().get())
    {
        cout << Context::current()->pop_rptr()->to_str();
    }
    Context::current()->push(NoneObject::one());
});

REGISTER_BUILTIN(println,
{
    if (Context::current()->top_rptr() != NoneObject::one().get())
    {
        cout << Context::current()->pop_rptr()->to_str() << endl;
    }
    Context::current()->push(NoneObject::one());
});

REGISTER_BUILTIN(input,
{
    String line;
    std::getline(cin, line);
    Context::current()->push(make_shared<StringObject>(line));
});

REGISTER_BUILTIN(exit,
{
    exit(0);
    Context::current()->push(NoneObject::one());
});

REGISTER_BUILTIN(time,
{
    time_t result = time(nullptr);
    Context::current()->push(make_shared<IntegerObject>(result));
});

REGISTER_BUILTIN(str,
{
    Context::current()->push(make_shared<StringObject>(Context::current()->pop_rptr()->to_str()));
});

REGISTER_BUILTIN(type,
{
    Context::current()->push(Context::current()->pop_rptr()->type());
});
}
