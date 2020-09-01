#pragma once

#include "ast.hpp"
#include "instruction.hpp"

#include "../objects/object.hpp"

#include <map>
#include <vector>
#include <iostream>
#include <filesystem>
#include <type_traits>

namespace anole
{
class Code
{
  public:
    friend class Collector;

    Code(String from = "<stdin>");

    const String &from()
    {
        return from_;
    }

    std::map<Size, AST::Position>
    &mapping()
    {
        return mapping_;
    }

    void mapping(const AST::Position &pos)
    {
        if (pos.first != 0)
        {
            mapping_[instructions_.size()] = pos;
        }
    }

    Instruction &ins_at(Size i)
    {
        return instructions_[i];
    }

    Opcode &opcode_at(Size i)
    {
        return instructions_[i].opcode;
    }

    std::any &oprand_at(Size i)
    {
        return instructions_[i].oprand;
    }

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

    Size size();
    void push_break(Size ind);
    void set_break_to(Size ind, Size base);
    void push_continue(Size ind);
    void set_continue_to(Size ind, Size base);
    bool check();
    ObjectSPtr load_const(Size ind);

    template<typename O, typename T>
    Size create_const(String key, T value)
    {
        static_assert(std::is_base_of_v<Object, O>);

        if (constants_map_.count(key))
        {
            return constants_map_[key];
        }
        else
        {
            constants_map_[key] = constants_.size();
            constants_literals_.push_back(key);
            constants_.push_back(std::make_shared<O>(value));
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
    std::map<Size, AST::Position> mapping_;

    std::vector<Instruction> instructions_;
    // these two should be checked is empty or not
    std::vector<Size> breaks_, continues_;
    std::vector<String> constants_literals_;

    std::map<String, Size> constants_map_;
    std::vector<ObjectSPtr> constants_;
};
}
