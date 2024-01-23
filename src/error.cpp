#include <lisp/error.hpp>

namespace lisp
{
  error::error(
    const std::string& message,
    std::optional<int> line,
    std::optional<int> column
  )
    : m_message(message)
    , m_line(line)
    , m_column(column) {}

  std::ostream&
  operator<<(std::ostream& os, const error& e)
  {
    if (const auto& line = e.line())
    {
      os << *line << ':';
      if (const auto& column = e.column())
      {
        os << *column << ':';
      }
      os << ' ';
    }
    os << e.message();

    return os;
  }
}
