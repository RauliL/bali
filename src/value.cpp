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
  to_atom(
    const value::ptr& value,
    const std::shared_ptr<class scope>& scope
  )
  {
    const auto result = scope ? eval(value, scope) : value;

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
  to_bool(
    const value::ptr& value,
    const std::shared_ptr<class scope>& scope
  )
  {
    const auto result = scope ? eval(value, scope) : value;

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
  to_list(
    const value::ptr& value,
    const std::shared_ptr<class scope>& scope
  )
  {
    const auto result = scope ? eval(value, scope) : value;

    if (result && result->type() == value::type::list)
    {
      return std::static_pointer_cast<value::list>(result)->elements();
    }

    throw error(
      "Value is not a list.",
      value ? value->line() : std::nullopt,
      value ? value->column() : std::nullopt
    );
  }

  double
  to_number(
    const value::ptr& value,
    const std::shared_ptr<class scope>& scope
  )
  {
    const auto result = scope ? eval(value, scope) : value;

    if (result && result->type() == value::type::atom)
    {
      const auto symbol = std::static_pointer_cast<value::atom>(
        result
      )->symbol();

      if (utils::is_number(symbol))
      {
        return std::stod(symbol);
      }
    }

    throw error(
      "Value is not a number.",
      value ? value->line() : std::nullopt,
      value ? value->column() : std::nullopt
    );
  }

  static std::string
  list_to_string(const std::shared_ptr<value::list>& list)
  {
    const auto& elements = list->elements();
    const auto size = elements.size();
    std::string result('(', 1);

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

  std::string
  to_string(const value::ptr& value)
  {
    if (!value)
    {
      return "nil";
    }

    switch (value->type())
    {
      case value::type::atom:
        return std::static_pointer_cast<value::atom>(value)->symbol();

      case value::type::list:
        return list_to_string(std::static_pointer_cast<value::list>(value));
    }

    return "<unknown>";
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
  operator<<(std::ostream& os, const value::ptr& value)
  {
    os << to_string(value);

    return os;
  }
}
