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
REGISTER_BUILTIN(eval, 1,
{
    auto str = dynamic_pointer_cast<StringObject>(args[0]);
    istringstream ss{"return " + str->to_str() + ";"};
    auto new_frame = make_shared<Frame>(theCurrentFrame,
        theCurrentFrame->scope());
    auto code = make_shared<Code>();
    Parser(ss).gen_statement()->codegen(*code);
    new_frame->execute_code(code);
    return theCurrentFrame->pop();
});

REGISTER_BUILTIN(call_with_current_continuation, 1,
{
    auto func = dynamic_pointer_cast<FunctionObject>(args[0]);
    auto cont_obj = make_shared<ContObject>(theCurrentFrame);
    auto new_frame = make_shared<Frame>(
        theCurrentFrame, func->scope());
    new_frame->push(cont_obj);
    new_frame->execute_code(func->code(), func->base());
    return theCurrentFrame->pop();
});

REGISTER_BUILTIN(id, 1,
{
    return make_shared<IntegerObject>(reinterpret_cast<int64_t>(args[0].get()));
})

REGISTER_BUILTIN(print, 1,
{
    cout << args[0]->to_str();
    return theNone;
});

REGISTER_BUILTIN(println, 1,
{
    cout << args[0]->to_str() << endl;
    return theNone;
});

REGISTER_BUILTIN(input, 0,
{
    string line;
    std::getline(cin, line);
    return make_shared<StringObject>(line);
});

REGISTER_BUILTIN(exit, 0,
{
    exit(0);
    return theNone;
});

REGISTER_BUILTIN(time, 0,
{
    time_t result = time(nullptr);
    return make_shared<IntegerObject>(result);
});
}
