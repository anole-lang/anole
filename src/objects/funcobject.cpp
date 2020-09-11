#include "objects.hpp"

#include "../runtime/runtime.hpp"
#include "../compiler/compiler.hpp"

#define OPRAND(T) any_cast<const T &>(Context::current()->oprand())

using namespace std;

namespace anole
{
FunctionObject::FunctionObject(SPtr<Scope> pre_scope,
    SPtr<Code> code, Size base,
    Size parameter_num)
  : Object(ObjectType::Func)
  , scope_(std::make_shared<Scope>(pre_scope))
  , code_(code), base_(base)
  , parameter_num_(parameter_num)
{
    // ...
}

String FunctionObject::to_str()
{
    return "<function>"s;
}

Address FunctionObject::load_member(const String &name)
{
    return scope_->load_symbol(name);
}

void FunctionObject::call(Size num)
{
    Context::current() = make_shared<Context>(
        Context::current(), scope_, code_, base_
    );

    auto parameter_num = parameter_num_;
    auto arg_num = num;
    auto &pc = Context::current()->pc();
    while (arg_num && parameter_num)
    {
        switch (Context::current()->opcode())
        {
        case Opcode::Pack:
        {
            ++pc;
            auto list = Allocator<Object>::alloc<ListObject>();
            if (Context::current()->opcode() == Opcode::StoreRef)
            {
                while (arg_num)
                {
                    list->objects().push_back(Context::current()->pop_address());
                    --arg_num;
                }
            }
            // else equals StoreLocal
            else
            {
                while (arg_num)
                {
                    list->append(Context::current()->pop_ptr());
                    --arg_num;
                }
            }
            Context::current()->scope()
                ->create_symbol(OPRAND(String))
                    ->bind(list)
            ;
        }
            ++pc;
            --parameter_num;
            break;

        case Opcode::StoreRef:
            Context::current()->scope()->create_symbol(OPRAND(String),
                Context::current()->pop_address()
            );
            ++pc;
            --arg_num;
            --parameter_num;
            break;

        case Opcode::StoreLocal:
            Context::current()->scope()
                ->create_symbol(OPRAND(String))
                    ->bind(Context::current()->pop_ptr())
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
        if (Context::current()->opcode() == Opcode::StoreRef
            || Context::current()->opcode() == Opcode::StoreLocal)
        {
            throw RuntimeError("missing the parameter named '" + OPRAND(String) + '\'');
        }
    }
}

void FunctionObject::collect(function<void(Scope *)> func)
{
    func(scope_.get());
}
}