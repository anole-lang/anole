#include <iostream>
#include "builtinfuncobject.hpp"

using namespace std;

namespace ice_language
{
REGISTER_BUILTIN("print", 1,
{
    cout << args[0]->to_str();
    return nullptr;
});
}
