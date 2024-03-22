#pragma once

#include <optional>

#include <bali/value.hpp>

namespace bali
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

  class function_return final
  {
  public:
    using value_type = bali::value::ptr;
    using reference = value_type&;
    using const_reference = const value_type&;

    explicit function_return(const_reference value = nullptr);
    function_return(const function_return&) = default;
    function_return(function_return&&) = default;
    function_return& operator=(const function_return&) = default;
    function_return& operator=(function_return&&) = default;

    inline const_reference value() const
    {
      return m_value;
    }

  private:
    value_type m_value;
  };
}
