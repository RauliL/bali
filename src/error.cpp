#include <peelo/unicode/encoding/utf8.hpp>

#include <bali/error.hpp>

namespace bali
{
  error::error(
    const std::u32string& message,
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
    os << peelo::unicode::encoding::utf8::encode(e.message());

    return os;
  }

  function_return::function_return(const_reference value)
    : m_value(value) {}
}
