#include <iostream>
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
}
