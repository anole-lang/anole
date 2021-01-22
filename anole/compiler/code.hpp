#ifndef __ANOLE_COMPILER_CODE_HPP__
#define __ANOLE_COMPILER_CODE_HPP__

#include "ast.hpp"
#include "instruction.hpp"

#include "../objects/object.hpp"
#include "../runtime/allocator.hpp"

#include <map>
#include <vector>
#include <iostream>
#include <filesystem>
#include <type_traits>

namespace anole
{
class Code
{
    friend class Collector;

  public:
    Code(String from, const std::filesystem::path &path);
    Code(String from, SPtr<std::filesystem::path> path) noexcept;

    const String &from();
    const std::filesystem::path &path() const;

    std::map<Size, Location> &source_mapping();
    void locate(const Location &location);

    const Instruction &ins_at(Size i) const;
    const Opcode &opcode_at(Size i) const;
    const std::any &oprand_at(Size i) const;

    Size add_ins(Instruction ins);
    template<Opcode op = Opcode::PlaceHolder>
    Size add_ins()
    {
        instructions_.push_back({ op });
        return instructions_.size() - 1;
    }

    template<Opcode op, typename T>
    Size add_ins(const T &value)
    {
        instructions_.push_back({ op, value });
        return instructions_.size() - 1;
    }

    template<Opcode op>
    void set_ins(Size ind)
    {
        instructions_[ind] = { op };
    }

    template<Opcode op, typename T>
    void set_ins(Size ind, const T &value)
    {
        instructions_[ind] = { op, value };
    }

    Size size() const noexcept;
    void push_break(Size ind);
    void set_break_to(Size ind, Size base);
    void push_continue(Size ind);
    void set_continue_to(Size ind, Size base);
    bool check();

    Object *load_const(Size ind);

    template<typename O, typename T>
    Size create_const(String key, T value)
    {
        static_assert(std::is_base_of_v<Object, O>);

        if (constants_mapping_.count(key))
        {
            return constants_mapping_[key];
        }
        else
        {
            constants_mapping_[key] = constants_.size();
            constants_literals_.push_back(key);
            /**
             * TODO:
             *
             * constants won't be deallocated now
             *  although the code is released
            */
            constants_.push_back(new O(value));
            return constants_.size() - 1;
        }
    }

    void print(const std::filesystem::path &path);
    void print(std::ostream &out = std::cout);

    void serialize(const std::filesystem::path &path);
    void serialize(std::ostream &out);

    bool unserialize(const std::filesystem::path &path);
    bool unserialize(std::ifstream &in);

    void clear();

  private:
    String from_;
    /**
     * SPtr for REPL
     *  because the main code from <stdin> should be with the working path
    */
    SPtr<std::filesystem::path> path_;
    std::map<Size, Location> source_mapping_;

    std::vector<Instruction> instructions_;
    // these two should be checked is empty or not
    std::vector<Size> breaks_, continues_;
    std::vector<String> constants_literals_;

    std::map<String, Size> constants_mapping_;
    std::vector<Object *> constants_;
};
}

#endif
