#pragma once

#include <iostream>
#include <optional>

namespace lisp
{
  class error final
  {
  public:
    explicit error(
      const std::string& message,
      std::optional<int> line = std::nullopt,
      std::optional<int> column = std::nullopt
    );
    error(const error&) = default;
    error(error&&) = delete;
    error& operator=(const error&) = default;
    error& operator=(error&&) = default;

    inline const std::string& message() const
    {
      return m_message;
    }

    inline const std::optional<int>& line() const
    {
      return m_line;
    }

    inline const std::optional<int>& column() const
    {
      return m_column;
    }

  private:
    std::string m_message;
    std::optional<int> m_line;
    std::optional<int> m_column;
  };

  std::ostream& operator<<(std::ostream&, const error&);
}
