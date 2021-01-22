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
        dynamic_cast<StringObject *>(theCurrContext->pop_ptr())->value() +
      ";"
    };

    /**
     * regard code from eval-expr as an extra area from the origin code
     *  so use the same path
    */
    auto code = std::make_shared<Code>("<eval>", theCurrContext->code_path());
    Parser(ss, "<eval>").gen_statement()->codegen(*code);
    theCurrContext = std::make_shared<Context>(
        theCurrContext, theCurrContext->scope(), code, -1
    );
    theCurrContext->scope() = theCurrContext->scope()->pre();
});

REGISTER_BUILTIN(call_with_current_continuation,
{
    if (theCurrContext->top_ptr()->is<ObjectType::Func>())
    {
        /**
         * TODO: check the collectable
        */
        auto func = reinterpret_cast<FunctionObject *>(theCurrContext->pop_ptr());
        // copy current context
        auto cont_obj = Allocator<Object>::alloc<ContObject>(theCurrContext);
        theCurrContext = std::make_shared<Context>(
            theCurrContext, func->scope(), func->code(), func->base()
        );
        // the base => StoreRef/StoreLocal
        theCurrContext->scope()
            ->create_symbol(std::any_cast<String>(
                theCurrContext->oprand())
            )->bind(cont_obj)
        ;
    }
    else if (theCurrContext->top_ptr()->is<ObjectType::Continuation>())
    {
        auto resume = theCurrContext->pop_ptr<ContObject>()->resume();
        auto cont_obj = Allocator<Object>::alloc<ContObject>(theCurrContext);
        theCurrContext = std::make_shared<Context>(resume);
        theCurrContext->push(cont_obj);
    }
    else
    {
        throw RuntimeError("err type as the argument for call/cc");
    }
});

REGISTER_BUILTIN(id,
{
    theCurrContext->push(
        Allocator<Object>::alloc<IntegerObject>(
            reinterpret_cast<int64_t>(
                theCurrContext->pop_ptr()
            )
        )
    );
});

REGISTER_BUILTIN(print,
{
    if (theCurrContext->top_ptr() != NoneObject::one())
    {
        std::cout << theCurrContext->pop_ptr()->to_str();
    }
    theCurrContext->push(NoneObject::one());
});

REGISTER_BUILTIN(println,
{
    if (theCurrContext->top_ptr() != NoneObject::one())
    {
        std::cout << theCurrContext->pop_ptr()->to_str() << std::endl;
    }
    theCurrContext->push(NoneObject::one());
});

REGISTER_BUILTIN(input,
{
    String line;
    std::getline(std::cin, line);
    theCurrContext->push(Allocator<Object>::alloc<StringObject>(line));
});

REGISTER_BUILTIN(exit,
{
    exit(0);
    theCurrContext->push(NoneObject::one());
});

REGISTER_BUILTIN(time,
{
    time_t result = std::time(nullptr);
    theCurrContext->push(Allocator<Object>::alloc<IntegerObject>(result));
});

REGISTER_BUILTIN(str,
{
    theCurrContext->push(Allocator<Object>::alloc<StringObject>(theCurrContext->pop_ptr()->to_str()));
});

REGISTER_BUILTIN(type,
{
    theCurrContext->push(theCurrContext->pop_ptr()->type());
});
}
