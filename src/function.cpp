#include <fstream>

#include <bali/error.hpp>
#include <bali/eval.hpp>
#include <bali/function.hpp>
#include <bali/parser.hpp>

namespace bali
{
  using builtin_function_map_type = std::unordered_map<
    std::string,
    builtin_function_callback_type
  >;
  using custom_function_map_type = std::unordered_map<
    std::string,
    std::shared_ptr<value::function>
  >;
  using compare_callback_type = bool(*)(double, double);

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
    compare_callback_type callback,
    value::list::iterator& it,
    const value::list::iterator& end,
    const std::shared_ptr<class scope>& scope
  )
  {
    if (it != end)
    {
      const auto operand = to_number(*it++, scope);

      while (it != end)
      {
        if (!callback(operand, to_number(*it++, scope)))
        {
          return value::atom::make_bool(false);
        }
      }
    }

    return value::atom::make_bool(true);
  }

  static value::ptr
  function_add(
    value::list::iterator& it,
    const value::list::iterator& end,
    const std::shared_ptr<class scope>& scope
  )
  {
    double result = 0;

    while (it != end)
    {
      result += to_number(*it++, scope);
    }

    return value::atom::make_number(result);
  }

  static value::ptr
  function_substract(
    value::list::iterator& it,
    const value::list::iterator& end,
    const std::shared_ptr<class scope>& scope
  )
  {
    auto result = to_number(eat("-", it, end), scope);

    if (it != end)
    {
      do
      {
        result -= to_number(*it++, scope);
      }
      while (it != end);
    } else {
      result = -result;
    }

    return value::atom::make_number(result);
  }

  static value::ptr
  function_multiply(
    value::list::iterator& it,
    const value::list::iterator& end,
    const std::shared_ptr<class scope>& scope
  )
  {
    double result = 1;

    while (it != end)
    {
      result *= to_number(*it++, scope);
    }

    return value::atom::make_number(result);
  }

  static value::ptr
  function_divide(
    value::list::iterator& it,
    const value::list::iterator& end,
    const std::shared_ptr<class scope>& scope
  )
  {
    auto result = to_number(eat("/", it, end), scope);

    if (it == end)
    {
      throw error("/: Not enough arguments.");
    }
    do
    {
      const auto divider = to_number(*it++, scope);

      if (divider == 0)
      {
        throw error("/: Division by zero.");
      }
      result /= divider;
    }
    while (it != end);

    return value::atom::make_number(result);
  }

  static value::ptr
  function_eq(
    value::list::iterator& it,
    const value::list::iterator& end,
    const std::shared_ptr<class scope>& scope
  )
  {
    return compare(
      [](double a, double b)
      {
        return a == b;
      },
      it,
      end,
      scope
    );
  }

  static value::ptr
  function_lt(
    value::list::iterator& it,
    const value::list::iterator& end,
    const std::shared_ptr<class scope>& scope
  )
  {
    return compare(
      [](double a, double b)
      {
        return a < b;
      },
      it,
      end,
      scope
    );
  }

  static value::ptr
  function_gt(
    value::list::iterator& it,
    const value::list::iterator& end,
    const std::shared_ptr<class scope>& scope
  )
  {
    return compare(
      [](double a, double b)
      {
        return a > b;
      },
      it,
      end,
      scope
    );
  }

  static value::ptr
  function_lte(
    value::list::iterator& it,
    const value::list::iterator& end,
    const std::shared_ptr<class scope>& scope
  )
  {
    return compare(
      [](double a, double b)
      {
        return a <= b;
      },
      it,
      end,
      scope
    );
  }

  static value::ptr
  function_gte(
    value::list::iterator& it,
    const value::list::iterator& end,
    const std::shared_ptr<class scope>& scope
  )
  {
    return compare(
      [](double a, double b)
      {
        return a >= b;
      },
      it,
      end,
      scope
    );
  }

  static value::ptr
  function_length(
    value::list::iterator& it,
    const value::list::iterator& end,
    const std::shared_ptr<class scope>& scope
  )
  {
    const auto list = to_list(eat("length", it, end), scope);

    finish("length", it, end);

    return value::atom::make_number(list.size());
  }

  static value::ptr
  function_cons(
    value::list::iterator& it,
    const value::list::iterator& end,
    const std::shared_ptr<class scope>& scope
  )
  {
    const auto head = eval(eat("cons", it, end), scope);
    const auto tail = to_list(eat("cons", it, end), scope);
    value::list::container_type result;

    finish("cons", it, end);
    result.reserve(tail.size() + 1);
    result.push_back(head);
    result.insert(std::end(result), std::begin(tail), std::end(tail));

    return value::list::make(result);
  }

  static value::ptr
  function_car(
    value::list::iterator& it,
    const value::list::iterator& end,
    const std::shared_ptr<class scope>& scope
  )
  {
    const auto list = to_list(eat("car", it, end), scope);

    finish("car", it, end);
    if (list.size() > 0)
    {
      return list[0];
    }

    throw error("car: Empty list.");
  }

  static value::ptr
  function_cdr(
    value::list::iterator& it,
    const value::list::iterator& end,
    const std::shared_ptr<class scope>& scope
  )
  {
    const auto list = to_list(eat("cdr", it, end), scope);

    finish("cdr", it, end);
    if (list.size() > 0)
    {
      return value::list::make(
        value::list::container_type(
          std::begin(list) + 1,
          std::end(list)
        )
      );
    }

    throw error("cdr: Empty list.");
  }

  static value::ptr
  function_list(
    value::list::iterator& it,
    const value::list::iterator& end,
    const std::shared_ptr<class scope>& scope
  )
  {
    value::list::container_type result;

    while (it != end)
    {
      result.push_back(eval(*it++, scope));
    }

    return value::list::make(result);
  }

  static value::ptr
  function_append(
    value::list::iterator& it,
    const value::list::iterator& end,
    const std::shared_ptr<class scope>& scope
  )
  {
    value::list::container_type result;

    while (it != end)
    {
      const auto list = to_list(*it++, scope);

      result.insert(std::end(result), std::begin(list), std::end(list));
    }

    return value::list::make(result);
  }

  static value::ptr
  function_not(
    value::list::iterator& it,
    const value::list::iterator& end,
    const std::shared_ptr<class scope>& scope
  )
  {
    const auto condition = to_bool(eat("not", it, end), scope);

    finish("not", it, end);

    return value::atom::make_bool(!condition);
  }

  static value::ptr
  function_and(
    value::list::iterator& it,
    const value::list::iterator& end,
    const std::shared_ptr<class scope>& scope
  )
  {
    while (it != end)
    {
      if (!to_bool(*it++, scope))
      {
        return value::atom::make_bool(false);
      }
    }

    return value::atom::make_bool(true);
  }

  static value::ptr
  function_or(
    value::list::iterator& it,
    const value::list::iterator& end,
    const std::shared_ptr<class scope>& scope
  )
  {
    while (it != end)
    {
      if (to_bool(*it++, scope))
      {
        return value::atom::make_bool(true);
      }
    }

    return value::atom::make_bool(false);
  }

  static value::ptr
  function_if(
    value::list::iterator& it,
    const value::list::iterator& end,
    const std::shared_ptr<class scope>& scope
  )
  {
    const auto condition = eat("if", it, end);
    const auto then_value = eat("if", it, end);
    value::ptr else_value;

    if (it != end)
    {
      else_value = *it++;
    }
    finish("if", it, end);

    return eval(to_bool(condition, scope) ? then_value : else_value, scope);
  }

  static value::ptr
  function_setq(
    value::list::iterator& it,
    const value::list::iterator& end,
    const std::shared_ptr<class scope>& scope
  )
  {
    const auto name = to_atom(eat("setq", it, end), scope);
    const auto value = eval(eat("setq", it, end), scope);

    finish("setq", it, end);
    scope->set(name, value);

    return value;
  }

  static value::ptr
  function_let(
    value::list::iterator& it,
    const value::list::iterator& end,
    const std::shared_ptr<class scope>& scope
  )
  {
    const auto variable_list = to_list(eat("let", it, end), nullptr);
    auto new_scope = std::make_shared<class scope>(scope);
    value::ptr return_value;

    for (const auto& entry : variable_list)
    {
      if (entry && entry->type() == value::type::list)
      {
        const auto& pair = std::static_pointer_cast<value::list>(
          entry
        )->elements();

        if (pair.size() != 2)
        {
          throw error(
            "Malformed `let` binding.",
            entry->line(),
            entry->column()
          );
        }
        new_scope->let(to_atom(pair[0], nullptr), eval(pair[1], scope));
      } else {
        new_scope->let(to_atom(entry, nullptr), nullptr);
      }
    }
    while (it != end)
    {
      return_value = eval(*it++, new_scope);
    }

    return return_value;
  }

  static value::ptr
  function_quote(
    value::list::iterator& it,
    const value::list::iterator& end,
    const std::shared_ptr<class scope>&
  )
  {
    const auto result = eat("quote", it, end);

    finish("quote", it, end);

    return result;
  }

  static value::ptr
  function_apply(
    value::list::iterator& it,
    const value::list::iterator& end,
    const std::shared_ptr<class scope>& scope
  )
  {
    const auto argument = eval(eat("apply", it, end), scope);
    const auto args = to_list(eat("apply", it, end), scope);
    auto begin = std::begin(args);

    finish("apply", it, end);
    if (argument && argument->type() == value::type::function)
    {
      return std::static_pointer_cast<value::function>(argument)->call(
        args,
        scope
      );
    }

    return call_function(
      to_atom(argument, nullptr),
      begin,
      std::end(args),
      scope
    );
  }

  static value::ptr
  function_defun(
    value::list::iterator& it,
    const value::list::iterator& end,
    const std::shared_ptr<scope>& scope
  )
  {
    const auto name = to_atom(eat("defun", it, end), scope);
    const auto raw_parameters = to_list(eat("defun", it, end), nullptr);
    std::vector<std::string> parameters;
    const auto expression = eat("defun", it, end);

    finish("defun", it, end);
    parameters.reserve(raw_parameters.size());
    for (const auto& parameter : raw_parameters)
    {
      parameters.push_back(to_atom(parameter, nullptr));
    }

    return define_custom_function(name, parameters, expression);
  }

  static value::ptr
  function_return_(
    value::list::iterator& it,
    const value::list::iterator& end,
    const std::shared_ptr<scope>& scope
  )
  {
    value::ptr return_value;

    if (it != end)
    {
      return_value = eval(*it++, scope);
    }
    finish("return", it, end);

    throw function_return(return_value);
  }

  static value::ptr
  function_lambda(
    value::list::iterator& it,
    const value::list::iterator& end,
    const std::shared_ptr<scope>&
  )
  {
    const auto raw_parameters = to_list(eat("lambda", it, end), nullptr);
    std::vector<std::string> parameters;
    const auto expression = eat("lambda", it, end);

    finish("lambda", it, end);
    parameters.reserve(raw_parameters.size());
    for (const auto& parameter : raw_parameters)
    {
      parameters.push_back(to_atom(parameter, nullptr));
    }

    return value::function::make(parameters, expression);
  }

  static value::ptr
  function_load(
    value::list::iterator& it,
    const value::list::iterator& end,
    const std::shared_ptr<class scope>& scope
  )
  {
    const auto filename = to_atom(eat("load", it, end), scope);

    finish("load", it, end);
    if (auto file = std::ifstream(filename))
    {
      const auto source = std::string(
        std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>()
      );

      file.close();
      for (const auto& value : parser(source).parse())
      {
        eval(value, scope);
      }
    } else {
      throw error("Unable to open file `" + filename + "'.");
    }

    return nullptr;
  }

  static value::ptr
  function_write(
    value::list::iterator& it,
    const value::list::iterator& end,
    const std::shared_ptr<class scope>& scope
  )
  {
    const auto result = eat("write", it, end);

    finish("write", it, end);
    std::cout << eval(result, scope) << std::endl;

    return nullptr;
  }

  static const builtin_function_map_type builtin_function_map =
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

    // Variables.
    { "setq", function_setq },
    { "let", function_let },

    // Functions.
    { "apply", function_apply },
    { "defun", function_defun },
    { "lambda", function_lambda },
    { "return", function_return_ },

    // Misc stuff.
    { "quote", function_quote },
    { "load", function_load },
    { "write", function_write },
  };

  static custom_function_map_type custom_function_map;

  std::shared_ptr<value>
  call_function(
    const std::string& name,
    value::list::iterator& begin,
    const value::list::iterator& end,
    const std::shared_ptr<class scope>& scope,
    const std::optional<int>& line,
    const std::optional<int>& column
  )
  {
    {
      const auto it = custom_function_map.find(name);

      if (it != std::end(custom_function_map))
      {
        value::list::container_type arguments;

        arguments.reserve(end - begin);
        while (begin != end)
        {
          arguments.push_back(eval(*begin++, scope));
        }

        return it->second->call(arguments, scope);
      }
    }

    {
      const auto it = builtin_function_map.find(name);

      if (it != std::end(builtin_function_map))
      {
        return (*it->second)(begin, end, scope);
      }
    }

    throw error("Unrecognized function: `" + name + "'", line, column);
  }

  std::shared_ptr<value::function>
  define_custom_function(
    const std::string& name,
    const std::vector<std::string>& parameters,
    const value::ptr& expression
  )
  {
    const auto function = value::function::make(parameters, expression, name);

    custom_function_map[name] = function;

    return function;
  }
}
