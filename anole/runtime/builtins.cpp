#include "runtime.hpp"

#include "../error.hpp"
#include "../objects/objects.hpp"
#include "../compiler/compiler.hpp"

#include <ctime>
#include <sstream>
#include <iostream>

namespace anole
{
REGISTER_BUILTIN(eval,
{
    std::istringstream ss
    {
      "return " +
        dynamic_cast<StringObject *>(Context::current()->pop_ptr())->value() +
      ";"
    };

    auto code = std::make_shared<Code>("<eval>");
    Parser(ss, "<eval>").gen_statement()->codegen(*code);
    Context::current() = std::make_shared<Context>(
        Context::current(), Context::current()->scope(), code, -1
    );
    Context::current()->scope() = Context::current()->scope()->pre();
});

REGISTER_BUILTIN(call_with_current_continuation,
{
    if (Context::current()->top_ptr()->is<ObjectType::Func>())
    {
        /**
         * TODO: check the collectable
        */
        auto func = reinterpret_cast<FunctionObject *>(Context::current()->pop_ptr());
        // copy current context
        auto cont_obj = Allocator<Object>::alloc<ContObject>(Context::current());
        Context::current() = std::make_shared<Context>(
            Context::current(), func->scope(), func->code(), func->base()
        );
        // the base => StoreRef/StoreLocal
        Context::current()->scope()
            ->create_symbol(std::any_cast<String>(
                Context::current()->oprand())
            )->bind(cont_obj)
        ;
    }
    else if (Context::current()->top_ptr()->is<ObjectType::Continuation>())
    {
        auto resume = Context::current()->pop_ptr<ContObject>()->resume();
        auto cont_obj = Allocator<Object>::alloc<ContObject>(Context::current());
        Context::current() = std::make_shared<Context>(resume);
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
                Context::current()->pop_ptr()
            )
        )
    );
});

REGISTER_BUILTIN(print,
{
    if (Context::current()->top_ptr() != NoneObject::one())
    {
        std::cout << Context::current()->pop_ptr()->to_str();
    }
    Context::current()->push(NoneObject::one());
});

REGISTER_BUILTIN(println,
{
    if (Context::current()->top_ptr() != NoneObject::one())
    {
        std::cout << Context::current()->pop_ptr()->to_str() << std::endl;
    }
    Context::current()->push(NoneObject::one());
});

REGISTER_BUILTIN(input,
{
    String line;
    std::getline(std::cin, line);
    Context::current()->push(Allocator<Object>::alloc<StringObject>(line));
});

REGISTER_BUILTIN(exit,
{
    exit(0);
    Context::current()->push(NoneObject::one());
});

REGISTER_BUILTIN(time,
{
    time_t result = std::time(nullptr);
    Context::current()->push(Allocator<Object>::alloc<IntegerObject>(result));
});

REGISTER_BUILTIN(str,
{
    Context::current()->push(Allocator<Object>::alloc<StringObject>(Context::current()->pop_ptr()->to_str()));
});

REGISTER_BUILTIN(type,
{
    Context::current()->push(Context::current()->pop_ptr()->type());
});
}
