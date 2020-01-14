#pragma once

#include <map>
#include <string>
#include <vector>
#include <iostream>
#include <type_traits>
#include "helper.hpp"
#include "object.hpp"
#include "instruction.hpp"

namespace ice_language
{
class Code
{
  public:
    Code();

    template <Opcode op = Opcode::PlaceHolder>
    std::size_t add_ins()
    {
        instructions_.push_back({
            op
        });
        return instructions_.size() - 1;
    }

    template <Opcode op, typename T>
    std::size_t add_ins(T value)
    {
        instructions_.push_back({
            op, value
        });
        return instructions_.size() - 1;
    }

    template <Opcode op>
    void set_ins(std::size_t ind)
    {
        instructions_[ind] = {op};
    }

    template <Opcode op, typename T>
    void set_ins(std::size_t ind, T value)
    {
        instructions_[ind] = {
            op, value
        };
    }

    std::size_t size();
    std::vector<Instruction> &get_instructions();
    void push_break(std::size_t ind);
    void set_break_to(std::size_t ind, std::size_t base);
    void push_continue(std::size_t ind);
    void set_continue_to(std::size_t ind, std::size_t base);
    bool check();
    ObjectPtr load_const(std::size_t ind);

    template <typename O, typename T>
    std::size_t create_const(std::string key, T value)
    {
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

    void print(std::ostream &out = std::cout);
    void to_file(std::ostream &out);

  private:
    std::vector<Instruction> instructions_;
    // these two should be checked is empty or not
    std::vector<std::size_t> breaks_, continues_;
    std::vector<std::string> constants_literals_;

    std::map<std::string, size_t> constants_map_;
    std::vector<ObjectPtr> constants_;
};
}
