#ifndef __ANOLE_RUNTIME_SCOPE_HPP__
#define __ANOLE_RUNTIME_SCOPE_HPP__

#include "variable.hpp"
#include "allocator.hpp"

#include <map>

namespace anole
{
class Scope
{
    friend class Collector;

  public:
    Scope();
    Scope(SPtr<Scope> pre_scope);

    SPtr<Scope> &pre();

    Address create_symbol(const String &name);
    void create_symbol(const String &name, Object *);
    void create_symbol(const String &name, Address value);

    Address load_symbol(const String &name);
    Address load_builtin(const String &name);

    const std::map<String, Address> &symbols() const;

  private:
    Address find_symbol(const String &name);

    SPtr<Scope> pre_scope_;
    std::map<String, Address> symbols_;
};
}

#endif
