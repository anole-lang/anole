#include "context.hpp"
#include "funcobject.hpp"
#include "boolobject.hpp"
#include "listobject.hpp"

#define OPRAND(T) any_cast<const T &>(theCurrentContext->oprand())

using namespace std;

namespace anole
{
String FunctionObject::to_str()
{
    return "<function>"s;
}

ObjectPtr FunctionObject::ceq(ObjectPtr obj)
{
    return (this == obj.get()) ? theTrue : theFalse;
}

ObjectPtr FunctionObject::cne(ObjectPtr obj)
{
    return (this != obj.get()) ? theTrue : theFalse;
}

Address FunctionObject::load_member(const String &name)
{
    return scope_->load_symbol(name);
}

void FunctionObject::call(Size num)
{
    theCurrentContext = make_shared<Context>(
        theCurrentContext, scope_, code_, base_
    );

    auto parameter_num = parameter_num_;
    auto arg_num = num;
    auto &pc = theCurrentContext->pc();
    while (arg_num && parameter_num)
    {
        switch (theCurrentContext->opcode())
        {
        case Opcode::Pack:
        {
            ++pc;
            auto list = make_shared<ListObject>();
            if (theCurrentContext->opcode() == Opcode::StoreRef)
            {
                while (arg_num)
                {
                    list->objects().push_back(theCurrentContext->pop_address());
                    --arg_num;
                }
            }
            // else equals StoreLocal
            else
            {
                while (arg_num)
                {
                    list->append(theCurrentContext->pop());
                    --arg_num;
                }
            }
            *theCurrentContext->scope()
                ->create_symbol(OPRAND(String))
                    = list
            ;
        }
            ++pc;
            --parameter_num;
            break;

        case Opcode::StoreRef:
            theCurrentContext->scope()->create_symbol(OPRAND(String),
                theCurrentContext->pop_address()
            );
            ++pc;
            --arg_num;
            --parameter_num;
            break;

        case Opcode::StoreLocal:
            *theCurrentContext->scope()
                ->create_symbol(OPRAND(String))
                    = theCurrentContext->pop()
            ;
            ++pc;
            --arg_num;
            --parameter_num;
            break;

        case Opcode::LambdaDecl:
        {
            using type = pair<Size, Size>;
            pc = (OPRAND(type)).second;
        }
            break;
        case Opcode::ThunkDecl:
            pc = OPRAND(Size);
            break;

        default:
            ++pc;
            break;
        }
    }

    /**
     * if there were some arguments unused
     *  means that there are too much arguments
    */
    if (arg_num)
    {
        throw RuntimeError("function takes " + to_string(parameter_num_) + " arguments but " + to_string(num) + " were given");
    }
    /**
     * if there were some parameters not meeting arguments
     *  check whether they have default values or not
     *
     * we can check this because parameters without default value
     *  cannot follow parameters with default value,
     *  so if the following instruction is StoreRef/StoreLocal
     *  means that the arguments is less than the function need
    */
    else if (parameter_num)
    {
        if (theCurrentContext->opcode() == Opcode::StoreRef
            || theCurrentContext->opcode() == Opcode::StoreLocal)
        {
            throw RuntimeError("missing the parameter named '" + OPRAND(String) + '\'');
        }
    }
}
}
