#include <fstream>
#include <unordered_map>

#include <bali/error.hpp>
#include <bali/parser.hpp>

namespace bali
{
  using callback_type = value::ptr(*)(
    value::list::iterator&,
    const value::list::iterator&
  );
  using function_map_type = std::unordered_map<std::string, callback_type>;
  using compare_callback_type = bool(*)(double, double);

  static inline function_map_type::const_iterator find_function(
    const std::string& name
  );
  static inline function_map_type::const_iterator function_map_end();

  static inline value::ptr
  eat(
    const char* function,
    value::list::iterator& it,
    const value::list::iterator& end
  )
  {
    if (it != end)
    {
      return *it++;
    }

    throw error(std::string(function) + ": Not enough arguments.");
  }

  static inline void
  finish(
    const char* function,
    value::list::iterator& it,
    const value::list::iterator& end
  )
  {
    if (it != end)
    {
      throw error(std::string(function) + ": Too many arguments.");
    }
  }

  static inline value::ptr
  compare(
    const char* function,
    compare_callback_type callback,
    value::list::iterator& it,
    const value::list::iterator& end
  )
  {
    if (it != end)
    {
      const auto operand = to_number(*it++);

      while (it != end)
      {
        if (!callback(operand, to_number(*it++)))
        {
          return make_bool(false);
        }
      }
    }

    return make_bool(true);
  }

  static value::ptr
  function_add(
    value::list::iterator& it,
    const value::list::iterator& end
  )
  {
    double result = 0;

    while (it != end)
    {
      result += to_number(*it++);
    }

    return make_number(result);
  }

  static value::ptr
  function_substract(
    value::list::iterator& it,
    const value::list::iterator& end
  )
  {
    auto result = to_number(eat("-", it, end));

    if (it != end)
    {
      do
      {
        result -= to_number(*it++);
      }
      while (it != end);
    } else {
      result = -result;
    }

    return make_number(result);
  }

  static value::ptr
  function_multiply(
    value::list::iterator& it,
    const value::list::iterator& end
  )
  {
    double result = 1;

    while (it != end)
    {
      result *= to_number(*it++);
    }

    return make_number(result);
  }

  static value::ptr
  function_divide(
    value::list::iterator& it,
    const value::list::iterator& end
  )
  {
    auto result = to_number(eat("/", it, end));

    if (it == end)
    {
      throw error("/: Not enough arguments.");
    }
    do
    {
      const auto divider = to_number(*it++);

      if (divider == 0)
      {
        throw error("/: Division by zero.");
      }
      result /= divider;
    }
    while (it != end);

    return make_number(result);
  }

  static value::ptr
  function_eq(
    value::list::iterator& it,
    const value::list::iterator& end
  )
  {
    return compare(
      "=",
      [](double a, double b)
      {
        return a == b;
      },
      it,
      end
    );
  }

  static value::ptr
  function_lt(
    value::list::iterator& it,
    const value::list::iterator& end
  )
  {
    return compare(
      "<",
      [](double a, double b)
      {
        return a < b;
      },
      it,
      end
    );
  }

  static value::ptr
  function_gt(
    value::list::iterator& it,
    const value::list::iterator& end
  )
  {
    return compare(
      ">",
      [](double a, double b)
      {
        return a > b;
      },
      it,
      end
    );
  }

  static value::ptr
  function_lte(
    value::list::iterator& it,
    const value::list::iterator& end
  )
  {
    return compare(
      "<=",
      [](double a, double b)
      {
        return a <= b;
      },
      it,
      end
    );
  }

  static value::ptr
  function_gte(
    value::list::iterator& it,
    const value::list::iterator& end
  )
  {
    return compare(
      ">=",
      [](double a, double b)
      {
        return a >= b;
      },
      it,
      end
    );
  }

  static value::ptr
  function_length(
    value::list::iterator& it,
    const value::list::iterator& end
  )
  {
    return make_number(to_list(eat("length", it, end)).size());
  }

  static value::ptr
  function_cons(
    value::list::iterator& it,
    const value::list::iterator& end
  )
  {
    const auto head = eval(eat("cons", it, end));
    const auto tail = to_list(eat("cons", it, end));
    value::list::container_type result;

    result.reserve(tail.size() + 1);
    result.push_back(head);
    result.insert(std::end(result), std::begin(tail), std::end(tail));

    return make_list(result);
  }

  static value::ptr
  function_car(
    value::list::iterator& it,
    const value::list::iterator& end
  )
  {
    const auto list = to_list(eat("car", it, end));

    if (list.size() > 0)
    {
      return list[0];
    }

    throw error("car: Empty list.");
  }

  static value::ptr
  function_cdr(
    value::list::iterator& it,
    const value::list::iterator& end
  )
  {
    const auto list = to_list(eat("cdr", it, end));

    if (list.size() > 0)
    {
      return make_list(value::list::container_type(
        std::begin(list) + 1,
        std::end(list))
      );
    }

    throw error("cdr: Empty list.");
  }

  static value::ptr
  function_list(
    value::list::iterator& it,
    const value::list::iterator& end
  )
  {
    value::list::container_type result;

    while (it != end)
    {
      result.push_back(eval(*it++));
    }

    return make_list(result);
  }

  static value::ptr
  function_append(
    value::list::iterator& it,
    const value::list::iterator& end
  )
  {
    value::list::container_type result;

    while (it != end)
    {
      const auto list = to_list(*it++);

      result.insert(std::end(result), std::begin(list), std::end(list));
    }

    return make_list(result);
  }

  static value::ptr
  function_not(
    value::list::iterator& it,
    const value::list::iterator& end
  )
  {
    const auto condition = to_bool(eat("not", it, end));

    finish("not", it, end);

    return make_bool(!condition);
  }

  static value::ptr
  function_and(
    value::list::iterator& it,
    const value::list::iterator& end
  )
  {
    while (it != end)
    {
      if (!to_bool(*it++))
      {
        return make_bool(false);
      }
    }

    return make_bool(true);
  }

  static value::ptr
  function_or(
    value::list::iterator& it,
    const value::list::iterator& end
  )
  {
    while (it != end)
    {
      if (to_bool(*it++))
      {
        return make_bool(true);
      }
    }

    return make_bool(false);
  }

  static value::ptr
  function_if(
    value::list::iterator& it,
    const value::list::iterator& end
  )
  {
    const auto condition = eat("if", it, end);
    const auto then_value = eat("if", it, end);
    const auto else_value = eat("if", it, end);

    finish("if", it, end);

    return eval(to_bool(condition) ? then_value : else_value);
  }

  static value::ptr
  function_quote(
    value::list::iterator& it,
    const value::list::iterator& end
  )
  {
    const auto result = eat("quote", it, end);

    finish("quote", it, end);

    return result;
  }

  static value::ptr
  function_apply(
    value::list::iterator& it,
    const value::list::iterator& end
  )
  {
    const auto id = to_atom(eat("apply", it, end));
    const auto args = to_list(eat("apply", it, end));
    auto begin = std::begin(args);
    const auto function = find_function(id);

    finish("apply", it, end);
    if (function != function_map_end())
    {
      return function->second(begin, std::end(args));
    }

    throw error("apply: unrecognized function: `" + id + "'");
  }

  static value::ptr
  function_load(
    value::list::iterator& it,
    const value::list::iterator& end
  )
  {
    const auto filename = to_atom(eat("load", it, end));

    finish("load", it, end);

    std::ifstream file(filename);

    if (file.good())
    {
      const auto source = std::string(
        std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>()
      );

      file.close();
      for (const auto& value : parser(source).parse())
      {
        eval(value);
      }
    }

    return nullptr;
  }

  static value::ptr
  function_write(
    value::list::iterator& it,
    const value::list::iterator& end
  )
  {
    const auto result = eat("write", it, end);

    finish("write", it, end);
    std::cout << eval(result) << std::endl;

    return nullptr;
  }

  static const function_map_type function_map =
  {
    // Arithmetic functions.
    { "+", function_add },
    { "-", function_substract },
    { "*", function_multiply },
    { "/", function_divide },

    // Comparison functions.
    { "=", function_eq },
    { "<", function_lt },
    { ">", function_gt },
    { "<=", function_lte },
    { ">=", function_gte },

    // List functions.
    { "length", function_length },
    { "cons", function_cons },
    { "car", function_car },
    { "cdr", function_cdr },
    { "list", function_list },
    { "append", function_append },

    // Conditions.
    { "not", function_not },
    { "and", function_and },
    { "or", function_or },
    { "if", function_if },

    // Misc stuff.
    { "quote", function_quote },
    { "apply", function_apply },
    { "load", function_load },
    { "write", function_write },
  };

  static inline function_map_type::const_iterator
  find_function(const std::string& name)
  {
    return function_map.find(name);
  }

  static inline function_map_type::const_iterator
  function_map_end()
  {
    return std::end(function_map);
  }

  value::ptr
  eval(const value::ptr& value)
  {
    if (!value)
    {
      return nullptr;
    }
    else if (value->type() == value::type::atom)
    {
      return !std::static_pointer_cast<value::atom>(
        value
      )->symbol().compare("nil")
        ? nullptr
        : value;
    } else {
      const auto& elements = std::static_pointer_cast<value::list>(
        value
      )->elements();

      if (elements.size() > 0)
      {
        auto begin = std::begin(elements);
        const auto end = std::end(elements);
        const auto id = to_atom(*begin++);
        const auto it = find_function(id);

        if (it != function_map.end())
        {
          return it->second(begin, end);
        }

        throw error(
          "Unrecognized function: `" + id + "'",
          value->line(),
          value->column()
        );
      }

      return value;
    }
  }
}
