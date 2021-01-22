#include "objects.hpp"

#include "../runtime/runtime.hpp"
#include "../compiler/compiler.hpp"

#define OPRAND(T) std::any_cast<const T &>(theCurrContext->oprand())

namespace anole
{
FunctionObject::FunctionObject(SPtr<Scope> pre_scope,
    SPtr<Code> code, Size base, Size parameter_num)
  : Object(ObjectType::Func)
  , scope_(std::make_shared<Scope>(pre_scope))
  , code_(code), base_(base)
  , parameter_num_(parameter_num)
{
    // ...
}

SPtr<Scope> FunctionObject::scope()
{
    return scope_;
}

SPtr<Code> FunctionObject::code()
{
    return code_;
}

Size FunctionObject::base()
{
    return base_;
}

String FunctionObject::to_str()
{
    return "<function>";
}

Address FunctionObject::load_member(const String &name)
{
    return scope_->load_symbol(name);
}

void FunctionObject::call(Size num)
{
    theCurrContext = std::make_shared<Context>(
        theCurrContext, scope_, code_, base_
    );

    auto parameter_num = parameter_num_;
    auto arg_num = num;
    auto &pc = theCurrContext->pc();
    while (arg_num && parameter_num)
    {
        switch (theCurrContext->opcode())
        {
        case Opcode::Pack:
        {
            ++pc;
            auto list = Allocator<Object>::alloc<ListObject>();
            if (theCurrContext->opcode() == Opcode::StoreRef)
            {
                while (arg_num)
                {
                    list->objects().push_back(theCurrContext->pop_address());
                    --arg_num;
                }
            }
            // else equals StoreLocal
            else
            {
                while (arg_num)
                {
                    list->append(theCurrContext->pop_ptr());
                    --arg_num;
                }
            }
            theCurrContext->scope()
                ->create_symbol(OPRAND(String))
                    ->bind(list)
            ;
        }
            ++pc;
            --parameter_num;
            break;

        case Opcode::StoreRef:
            theCurrContext->scope()->create_symbol(OPRAND(String),
                theCurrContext->pop_address()
            );
            ++pc;
            --arg_num;
            --parameter_num;
            break;

        case Opcode::StoreLocal:
            theCurrContext->scope()
                ->create_symbol(OPRAND(String))
                    ->bind(theCurrContext->pop_ptr())
            ;
            ++pc;
            --arg_num;
            --parameter_num;
            break;

        case Opcode::LambdaDecl:
        {
            using type = std::pair<Size, Size>;
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
        throw RuntimeError("function takes " + std::to_string(parameter_num_) + " arguments but " + std::to_string(num) + " were given");
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
        if (theCurrContext->opcode() == Opcode::StoreRef
            || theCurrContext->opcode() == Opcode::StoreLocal)
        {
            throw RuntimeError("missing the parameter named '" + OPRAND(String) + '\'');
        }
    }
}

bool FunctionObject::is_callable()
{
    return true;
}

void FunctionObject::collect(std::function<void(Scope *)> func)
{
    func(scope_.get());
}
}
