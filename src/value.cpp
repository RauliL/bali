#include <iostream>
#include <unordered_map>

#include <peelo/unicode/encoding/utf8.hpp>

#include <bali/error.hpp>
#include <bali/eval.hpp>

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
    using peelo::unicode::encoding::utf8::decode;
    char buffer[BUFSIZ];

    std::snprintf(buffer, BUFSIZ, "%g", value);

    return make(decode(buffer), line, column);
  }

  value::atom::atom(
    const std::u32string& symbol,
    const std::optional<int>& line,
    const std::optional<int>& column
  )
    : value::value(line, column)
    , m_symbol(symbol) {}

  value::list::list(
    const container_type& elements,
    const std::optional<int>& line,
    const std::optional<int>& column
  )
    : value::value(line, column)
    , m_elements(elements) {}

  std::u32string
  value::list::to_string() const
  {
    const auto size = m_elements.size();
    std::u32string result(1, U'(');

    for (value::list::container_type::size_type i = 0; i < size; ++i)
    {
      if (i > 0)
      {
        result += U' ';
      }
      result += value::to_string(m_elements[i]);
    }
    result += U')';

    return result;
  }

  value::function::function(
    const std::vector<std::u32string>& parameters,
    const value::ptr& expression,
    const std::optional<std::u32string>& name,
    const std::optional<int>& line,
    const std::optional<int>& column
  )
    : value::value(line, column)
    , m_parameters(parameters)
    , m_expression(expression)
    , m_name(name) {}

  static inline std::u32string
  get_function_name(const std::optional<std::u32string>& name)
  {
    return name ? *name : U"<anonymous>";
  }

  value::ptr
  value::function::call(
    const value::list::container_type& arguments,
    const std::shared_ptr<class scope>& scope
  ) const
  {
    const auto size = arguments.size();
    const auto function_scope =
      m_parameters.empty()
        ? scope
        : std::make_shared<class scope>(scope);

    if (size < m_parameters.size())
    {
      throw error(get_function_name(m_name) + U": Not enough arguments.");
    }
    else if (size > m_parameters.size())
    {
      throw error(get_function_name(m_name) + U": Too many arguments.");
    }
    for (value::list::size_type i = 0; i < size; ++i)
    {
      function_scope->let(m_parameters[i], arguments[i]);
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

  std::u32string
  value::function::to_string() const
  {
    std::u32string result(1, U'(');
    const auto parameters_size = m_parameters.size();

    if (m_name)
    {
      result += U"defun " + *m_name + U' ';
    } else {
      result += U"lambda ";
    }
    result += '(';
    for (std::vector<std::string>::size_type i = 0; i < parameters_size; ++i)
    {
      if (i > 0)
      {
        result += U' ';
      }
      result += m_parameters[i];
    }

    return result + U") " + value::to_string(m_expression) + U')';
  }

  std::ostream&
  operator<<(std::ostream& os, const value::ptr& value)
  {
    using peelo::unicode::encoding::utf8::encode;

    os << encode(value::to_string(value));

    return os;
  }
}
