#include <ctime>
#include <iostream>
#include "integerobject.hpp"
#include "builtinfuncobject.hpp"

using namespace std;

namespace ice_language
{
REGISTER_BUILTIN(print, 1,
{
    cout << args[0]->to_str();
    return nullptr;
});

REGISTER_BUILTIN(println, 1,
{
    cout << args[0]->to_str() << endl;
    return nullptr;
});

REGISTER_BUILTIN(exit, 0,
{
    exit(0);
    return nullptr;
});

REGISTER_BUILTIN(time, 0,
{
    time_t result = time(nullptr);
    return make_shared<IntegerObject>(result);
});
}
