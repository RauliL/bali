#include <iostream>
#include <unordered_map>

#include <bali/error.hpp>
#include <bali/eval.hpp>
#include <bali/utils.hpp>

#if !defined(BUFSIZ)
#  define BUFSIZ 1024
#endif

namespace bali
{
  value::value(
    const std::optional<int>& line,
    const std::optional<int>& column
  )
    : m_line(line)
    , m_column(column) {}

  std::shared_ptr<value::atom>
  value::atom::make_number(
    double value,
    const std::optional<int>& line,
    const std::optional<int>& column
  )
  {
    char buffer[BUFSIZ];

    std::snprintf(buffer, BUFSIZ, "%g", value);

    return make(buffer, line, column);
  }

  value::atom::atom(
    const std::string& symbol,
    const std::optional<int>& line,
    const std::optional<int>& column
  )
    : value::value(line, column)
    , m_symbol(symbol) {}

  value::cons::cons(
    const ptr& car,
    const ptr& cdr,
    const std::optional<int>& line,
    const std::optional<int>& column
  )
    : value::value(line, column)
    , m_car(car)
    , m_cdr(cdr) {}

  value::cons::size_type
  value::cons::length() const
  {
    if (m_cdr)
    {
      if (m_cdr->type() == type::cons)
      {
        return std::static_pointer_cast<cons>(m_cdr)->length() + 1;
      }

      return 2;
    }

    return 1;
  }

  std::string
  value::cons::to_string() const
  {
    return "(" +
      value::to_string(m_car) +
      " . " +
      value::to_string(m_cdr) +
      ")";
  }

  value::list::list(
    const container_type& elements,
    const std::optional<int>& line,
    const std::optional<int>& column
  )
    : value::value(line, column)
    , m_elements(elements) {}

  std::string
  value::list::to_string() const
  {
    const auto size = m_elements.size();
    std::string result(1, '(');

    for (value::list::container_type::size_type i = 0; i < size; ++i)
    {
      if (i > 0)
      {
        result += ' ';
      }
      result += value::to_string(m_elements[i]);
    }
    result += ')';

    return result;
  }

  value::function::function(
    const std::vector<std::string>& parameters,
    const value::ptr& expression,
    const std::optional<std::string>& name,
    const std::optional<int>& line,
    const std::optional<int>& column
  )
    : value::value(line, column)
    , m_parameters(parameters)
    , m_expression(expression)
    , m_name(name) {}

  static inline std::string
  get_function_name(const std::optional<std::string>& name)
  {
    return name ? *name : "<anonymous>";
  }

  value::ptr
  value::function::call(
    value::list::iterator& begin,
    const value::list::iterator& end,
    const std::shared_ptr<class scope>& scope
  ) const
  {
    auto function_scope =
      m_parameters.empty()
        ? scope
        : std::make_shared<class scope>(scope);

    for (const auto& parameter : m_parameters)
    {
      if (begin >= end)
      {
        throw error(get_function_name(m_name) + ": Not enough arguments.");
      }
      function_scope->let(parameter, *begin++);
    }
    if (begin != end)
    {
      throw error(get_function_name(m_name) + ": Too many arguments.");
    }

    try
    {
      return eval(m_expression, function_scope);
    }
    catch (function_return& e)
    {
      return e.value();
    }
  }

  std::string
  value::function::to_string() const
  {
    std::string result(1, '(');
    const auto parameters_size = m_parameters.size();

    if (m_name)
    {
      result += "defun " + *m_name;
    } else {
      result += "lambda ";
    }
    result += '(';
    for (std::vector<std::string>::size_type i = 0; i < parameters_size; ++i)
    {
      if (i > 0)
      {
        result += ' ';
      }
      result += m_parameters[i];
    }

    return result;
  }

  std::ostream&
  operator<<(std::ostream& os, const value::ptr& value)
  {
    os << value::to_string(value);

    return os;
  }
}
