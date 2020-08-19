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
        reinterpret_cast<StringObject *>(Context::current()->pop())->value() +
      ";"
    };

    auto code = make_shared<Code>("<eval>");
    Parser(ss, "<eval>").gen_statement()->codegen(*code);
    Context::current() = Allocator<Context>::alloc(
        Context::current(), Context::current()->scope(), code, -1
    );
    Context::current()->scope() = Context::current()->scope()->pre();
});

REGISTER_BUILTIN(call_with_current_continuation,
{
    if (Context::current()->top()->is<ObjectType::Func>())
    {
        auto func = Context::current()->pop();
        auto ptr = reinterpret_cast<FunctionObject *>(func);
        auto cont_obj = Allocator<Object>::alloc<ContObject>(Context::current());
        Context::current() = Allocator<Context>::alloc(
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
        auto resume = reinterpret_cast<ContObject *>(Context::current()->pop())->resume();
        auto cont_obj = Allocator<Object>::alloc<ContObject>(Context::current());
        Context::current() = Allocator<Context>::alloc(resume);
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
        Allocator<Object>::alloc<IntegerObject>(
            reinterpret_cast<int64_t>(
                Context::current()->pop()))
    );
});

REGISTER_BUILTIN(print,
{
    if (Context::current()->top() != NoneObject::one())
    {
        cout << Context::current()->pop()->to_str();
    }
    Context::current()->push(NoneObject::one());
});

REGISTER_BUILTIN(println,
{
    if (Context::current()->top() != NoneObject::one())
    {
        cout << Context::current()->pop()->to_str() << endl;
    }
    Context::current()->push(NoneObject::one());
});

REGISTER_BUILTIN(input,
{
    String line;
    std::getline(cin, line);
    Context::current()->push(Allocator<Object>::alloc<StringObject>(line));
});

REGISTER_BUILTIN(exit,
{
    exit(0);
    Context::current()->push(NoneObject::one());
});

REGISTER_BUILTIN(time,
{
    time_t result = time(nullptr);
    Context::current()->push(Allocator<Object>::alloc<IntegerObject>(result));
});

REGISTER_BUILTIN(str,
{
    Context::current()->push(Allocator<Object>::alloc<StringObject>(Context::current()->pop()->to_str()));
});

REGISTER_BUILTIN(type,
{
    Context::current()->push(Context::current()->pop()->type());
});
}
