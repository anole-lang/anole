#include <ctime>
#include <sstream>
#include <iostream>
#include "code.hpp"
#include "frame.hpp"
#include "parser.hpp"
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
    auto str = dynamic_pointer_cast<StringObject>(theCurrentFrame->pop());
    istringstream ss{"return " + str->to_str() + ";"};
    auto code = make_shared<Code>();
    Parser(ss).gen_statement()->codegen(*code);
    theCurrentFrame->push(nullptr);
    theCurrentFrame = make_shared<Frame>(theCurrentFrame,
        theCurrentFrame->scope(), code, 0);
});

REGISTER_BUILTIN(call_with_current_continuation,
{
    if (dynamic_pointer_cast<FunctionObject>(theCurrentFrame->top()))
    {
        auto func = theCurrentFrame->pop<FunctionObject>();
        auto cont_obj = make_shared<ContObject>(theCurrentFrame);
        theCurrentFrame = make_shared<Frame>(
            theCurrentFrame, func->scope(), func->code(), func->base() - 1);
        theCurrentFrame->push(cont_obj);
    }
    else if (dynamic_pointer_cast<ContObject>(theCurrentFrame->top()))
    {
        auto resume_to = theCurrentFrame->pop<ContObject>()->resume_to();
        auto cont_obj = make_shared<ContObject>(theCurrentFrame);
        resume_to->push(cont_obj);
        theCurrentFrame = resume_to;
    }
    else
    {
        throw runtime_error("err type as the argument for call/cc");
    }
});

REGISTER_BUILTIN(id,
{
    theCurrentFrame->push(make_shared<IntegerObject>(reinterpret_cast<int64_t>(theCurrentFrame->pop().get())));
})

REGISTER_BUILTIN(print,
{
    cout << theCurrentFrame->pop()->to_str();
    theCurrentFrame->push(theNone);
});

REGISTER_BUILTIN(println,
{
    cout << theCurrentFrame->pop()->to_str() << endl;
    theCurrentFrame->push(theNone);
});

REGISTER_BUILTIN(input,
{
    string line;
    std::getline(cin, line);
    theCurrentFrame->push(make_shared<StringObject>(line));
});

REGISTER_BUILTIN(exit,
{
    exit(0);
    theCurrentFrame->push(theNone);
});

REGISTER_BUILTIN(time,
{
    time_t result = time(nullptr);
    theCurrentFrame->push(make_shared<IntegerObject>(result));
});
}
