#pragma once

#include <bali/value.hpp>

namespace bali
{
  class parser final
  {
  public:
    explicit parser(const std::string& input, int line = 1);
    parser(const parser&) = delete;
    parser(parser&&) = delete;
    void operator=(const parser&) = delete;
    void operator=(parser&&) = delete;

    value::list::container_type parse();

  private:
    void skip_whitespace();
    value::ptr parse_value();
    bool eof() const;
    char read();
    bool peek(char input) const;
    bool peek_read(char input);
    bool peek_read(bool (*callback)(char));

  private:
    std::string::const_iterator m_pos;
    const std::string::const_iterator m_end;
    int m_line;
    int m_column;
  };
}
