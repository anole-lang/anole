#include <ctime>
#include <sstream>
#include <iostream>
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

using namespace std;

namespace ice_language
{
REGISTER_BUILTIN(eval,
{
    auto str = dynamic_pointer_cast<StringObject>(theCurrentContext->pop());
    istringstream ss{"return " + str->to_str() + ";"};
    auto code = make_shared<Code>("<eval>");
    Parser(ss, "<eval>").gen_statement()->codegen(*code);
    theCurrentContext = make_shared<Context>(theCurrentContext,
        theCurrentContext->scope(), code, 0);
});

REGISTER_BUILTIN(call_with_current_continuation,
{
    if (dynamic_pointer_cast<FunctionObject>(theCurrentContext->top()))
    {
        auto func = theCurrentContext->pop<FunctionObject>();
        auto cont_obj = make_shared<ContObject>(theCurrentContext);
        // base => LoadConst 0, so start with base + 1
        theCurrentContext = make_shared<Context>(
            theCurrentContext, func->scope(), func->code(), func->base() + 1);
        *theCurrentContext->scope()->create_symbol(any_cast<string>(theCurrentContext->oprand()))
            = cont_obj;
    }
    else if (dynamic_pointer_cast<ContObject>(theCurrentContext->top()))
    {
        auto resume = theCurrentContext->pop<ContObject>()->resume();
        auto cont_obj = make_shared<ContObject>(theCurrentContext);
        theCurrentContext = make_shared<Context>(resume);
        theCurrentContext->push(cont_obj);
    }
    else
    {
        throw RuntimeError("err type as the argument for call/cc");
    }
});

REGISTER_BUILTIN(id,
{
    theCurrentContext->push(make_shared<IntegerObject>(reinterpret_cast<int64_t>(theCurrentContext->pop().get())));
})

REGISTER_BUILTIN(print,
{
    if (theCurrentContext->top() != theNone)
    {
        cout << theCurrentContext->pop()->to_str();
    }
    theCurrentContext->push(theNone);
});

REGISTER_BUILTIN(println,
{
    if (theCurrentContext->top() != theNone)
    {
        cout << theCurrentContext->pop()->to_str() << endl;
    }
    theCurrentContext->push(theNone);
});

REGISTER_BUILTIN(input,
{
    string line;
    std::getline(cin, line);
    theCurrentContext->push(make_shared<StringObject>(line));
});

REGISTER_BUILTIN(exit,
{
    exit(0);
    theCurrentContext->push(theNone);
});

REGISTER_BUILTIN(time,
{
    time_t result = time(nullptr);
    theCurrentContext->push(make_shared<IntegerObject>(result));
});
}
