#include <functional>
#include <iostream>
#include <unordered_map>

#include <lisp/error.hpp>
#include <lisp/value.hpp>

#if !defined(BUFSIZ)
#  define BUFSIZ 1024
#endif

namespace lisp
{
  static inline bool
  is_number(const std::string& input)
  {
    const auto length = input.length();
    std::string::size_type start;
    bool dot_seen = false;

    if (!length)
    {
      return false;
    }

    if (input[0] == '+' || input[1] == '-')
    {
      start = 1;
      if (length < 2)
      {
        return false;
      }
    } else {
      start = 0;
    }

    for (std::string::size_type i = start; i < length; ++i)
    {
      const auto& c = input[i];

      if (c == '.')
      {
        if (dot_seen || i == start || i + 1 > length)
        {
          return false;
        }
        dot_seen = true;
      }
      else if (!std::isdigit(c))
      {
        return false;
      }
    }

    return true;
  }

  value::value(
    const std::optional<int>& line,
    const std::optional<int>& column
  )
    : m_line(line)
    , m_column(column) {}

  value::atom::atom(
    const std::string& symbol,
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

  std::string
  to_atom(const std::shared_ptr<class value>& value)
  {
    const auto result = eval(value);

    if (result && result->type() == value::type::atom)
    {
      return std::static_pointer_cast<value::atom>(value)->symbol();
    }

    throw error(
      "Value is not an atom.",
      value ? value->line() : std::nullopt,
      value ? value->column() : std::nullopt
    );
  }

  bool
  to_bool(const std::shared_ptr<class value>& value)
  {
    const auto result = eval(value);

    if (!result)
    {
      return false;
    }
    else if (result->type() == value::type::atom)
    {
      return std::static_pointer_cast<value::atom>(
        result
      )->symbol().compare("nil") != 0;
    }

    return true;
  }

  value::list::container_type
  to_list(const std::shared_ptr<class value>& value)
  {
    const auto result = eval(value);

    if (result && result->type() == value::type::list)
    {
      return std::static_pointer_cast<value::list>(result)->elements();
    }

    throw error(
      "Value is not a list.",
      value? value->line() : std::nullopt,
      value? value->column() : std::nullopt
    );
  }

  double
  to_number(const std::shared_ptr<class value>& value)
  {
    const auto result = eval(value);

    if (result && result->type() == value::type::atom)
    {
      const auto symbol = std::static_pointer_cast<value::atom>(
        result
      )->symbol();

      if (is_number(symbol))
      {
        return std::stod(symbol);
      }
    }

    throw error(
      "Value is not a number.",
      value? value->line() : std::nullopt,
      value? value->column() : std::nullopt
    );
  }

  std::string
  to_string(const std::shared_ptr<class value>& value)
  {
    if (value)
    {
      if (value->type() == value::type::atom)
      {
        return std::static_pointer_cast<value::atom>(value)->symbol();
      } else {
        std::string result;
        const auto elements = std::static_pointer_cast<value::list>(
          value
        )->elements();
        const auto size = elements.size();

        result += '(';
        for (value::list::container_type::size_type i = 0; i < size; ++i)
        {
          if (i > 0)
          {
            result += ' ';
          }
          result += to_string(elements[i]);
        }
        result += ')';

        return result;
      }
    }

    return "nil";
  }

  std::shared_ptr<value::atom>
  make_bool(
    bool value,
    const std::optional<int>& line,
    const std::optional<int>& column
  )
  {
    return value ? std::make_shared<value::atom>("true") : nullptr;
  }

  std::shared_ptr<value::atom>
  make_number(
    double value,
    const std::optional<int>& line,
    const std::optional<int>& column
  )
  {
    char buffer[BUFSIZ];

    std::snprintf(buffer, BUFSIZ, "%g", value);

    return std::make_shared<value::atom>(buffer, line, column);
  }

  std::shared_ptr<value::atom>
  make_atom(
    const std::string& value,
    const std::optional<int>& line,
    const std::optional<int>& column
  )
  {
    return std::make_shared<value::atom>(value, line, column);
  }

  std::shared_ptr<value::list>
  make_list(
    const value::list::container_type& elements,
    const std::optional<int>& line,
    const std::optional<int>& column
  )
  {
    return std::make_shared<value::list>(elements, line, column);
  }

  std::ostream&
  operator<<(std::ostream& os, const std::shared_ptr<class value>& value)
  {
    os << to_string(value);

    return os;
  }
}
