#include "code.hpp"
#include "error.hpp"
#include "parser.hpp"
#include "context.hpp"
#include "noneobject.hpp"
#include "boolobject.hpp"
#include "funcobject.hpp"
#include "contobject.hpp"
#include "stringobject.hpp"
#include "integerobject.hpp"
#include "builtinfuncobject.hpp"

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
        reinterpret_cast<StringObject *>(Context::current()->pop().get())->value() +
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
    if (Context::current()->top()->is<ObjectType::Func>())
    {
        auto func = Context::current()->pop();
        auto ptr = reinterpret_cast<FunctionObject *>(func.get());
        auto cont_obj = make_shared<ContObject>(Context::current());
        Context::current() = make_shared<Context>(
            Context::current(), ptr->scope(), ptr->code(), ptr->base()
        );
        // the base => StoreRef/StoreLocal
        Context::current()->scope()
            ->create_symbol(any_cast<String>(
                Context::current()->oprand()))
                    ->bind(cont_obj)
        ;
    }
    else if (Context::current()->top()->is<ObjectType::Cont>())
    {
        auto resume = reinterpret_cast<ContObject *>(Context::current()->pop().get())->resume();
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
                Context::current()->pop().get()))
    );
});

REGISTER_BUILTIN(print,
{
    if (Context::current()->top() != NoneObject::one().get())
    {
        cout << Context::current()->pop()->to_str();
    }
    Context::current()->push(NoneObject::one());
});

REGISTER_BUILTIN(println,
{
    if (Context::current()->top() != NoneObject::one().get())
    {
        cout << Context::current()->pop()->to_str() << endl;
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
    Context::current()->push(make_shared<StringObject>(Context::current()->pop()->to_str()));
});

REGISTER_BUILTIN(type,
{
    Context::current()->push(Context::current()->pop()->type());
});
}
