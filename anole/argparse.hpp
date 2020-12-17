#ifndef __ANOLE_ARGPARSE_HPP__
#define __ANOLE_ARGPARSE_HPP__

#include "base.hpp"

#include <any>
#include <map>
#include <vector>
#include <iostream>
#include <exception>
#include <functional>

namespace anole
{
class Argument
{
  public:
    using ActionType
        = std::function<std::any(const String &)>
    ;

    Argument() : action_([](const String &value) { return value; })
    {
        // ...
    }

    Argument &help(String info)
    {
        help_info_ = std::move(info);
        return *this;
    }

    template<typename T>
    Argument &default_value(const T &&value)
    {
        value_ = value;
        return *this;
    }

    template<typename T>
    Argument &implict_value(const T &&value)
    {
        implict_value_ = value;
        return *this;
    }

    Argument &action(ActionType action)
    {
        action_ = std::move(action);
        return *this;
    }

    void consume(const String &value)
    {
        value_ = action_(value);
    }

    void set_implict_value()
    {
        value_ = implict_value_;
    }

    bool is_implict()
    {
        return implict_value_.has_value();
    }

    template<typename T = String>
    T get()
    {
        return std::any_cast<T>(value_);
    }

    const String &help_info() const
    {
        return help_info_;
    }

  private:
    std::any value_;
    std::any implict_value_;
    String help_info_;
    ActionType action_;
};

class ArgumentParser
{
  public:
    ArgumentParser() = default;

    ArgumentParser(String program)
      : program_(std::move(program))
    {
        // ...
    }

    void parse(int argc, char *argv[])
    {
        if (program_.empty())
        {
            program_ = argv[0];
        }

        int i = 1;
        auto pit = positional_arguments_.begin();
        while (i < argc)
        {
            String value = argv[i++];
            if (value[0] != '-')
            {
                arguments_[pit++->first].consume(value);
            }
            else
            {
                auto &arg = arguments_[keys_[get_main(value)]];
                if (arg.is_implict())
                {
                    arg.set_implict_value();
                }
                else if (i < argc && argv[i][0] != '-')
                {
                    arg.consume(argv[i++]);
                }
                else
                {
                    throw;
                }
            }
        }
    }

    void print_help(std::ostream &out = std::cout)
    {
        out << "Usage: " << program_ << " [options]";

        for (auto &ind_key : positional_arguments_)
        {
            out << " " << ind_key.second;
        }

        out << "\n\nPositional arguments:" << std::endl;

        for (auto &ind_key : positional_arguments_)
        {
            out << ind_key.second << "\t"
                << arguments_[ind_key.first].help_info()
                << std::endl
            ;
        }

        out << "\nOptional arguments:" << std::endl;

        for (auto &ind_keys : optional_arguments_)
        {
            for (decltype(ind_keys.second.size()) i = 0;
                i < ind_keys.second.size(); ++i)
            {
                if (i) out << ", ";
                out << ind_keys.second[i];
            }
            out << "\t" << arguments_[ind_keys.first].help_info() << std::endl;
        }
    }

    template<typename ...Ts>
    Argument &add_argument(const String &key,
        const Ts & ...keys)
    {
        if (key.empty())
        {
            throw;
        }
        if (key[0] != '-')
        {
            return add_positional_argument(key);
        }
        auto k = arguments_.size();
        optional_arguments_[k].push_back(key);
        keys_[get_main(key)] = k;
        if constexpr (sizeof...(Ts))
        {
            add_optional_argument(keys...);
        }
        arguments_[k] = Argument();
        return arguments_[k];
    }

    template<typename V = String>
    V get(const String &key)
    {
        return arguments_[keys_[key]].get<V>();
    }

  private:
    Argument &add_positional_argument(
        const String &key)
    {
        auto k = arguments_.size();
        positional_arguments_[k] = key;
        keys_[positional_arguments_[k]] = k;
        arguments_[k] = Argument();
        return arguments_[k];
    }

    template<typename ...Ts>
    void add_optional_argument(
        const String &key,
        const Ts & ...keys)
    {
        auto k = arguments_.size();
        optional_arguments_[k].push_back(key);
        keys_[get_main(key)] = k;
        if constexpr (sizeof...(Ts) > 0)
        {
            add_positional_argument(keys...);
        }
    }

    // assume arg starts with '-'
    String get_main(const String &arg)
    {
        if (arg[1] == '-')
        {
            return arg.substr(2);
        }
        return arg.substr(1);
    }

    String program_;
    std::map<String, Size> keys_;
    std::map<Size, String> positional_arguments_;
    std::map<Size, std::vector<String>> optional_arguments_;
    std::map<Size, Argument> arguments_;
};
}

#endif
