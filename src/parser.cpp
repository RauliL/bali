#include <cctype>

#include <bali/error.hpp>
#include <bali/parser.hpp>

namespace bali
{
  static inline bool
  isspace(char c)
  {
    return std::isspace(c);
  }

  parser::parser(const std::string& input, int line)
    : m_pos(std::begin(input))
    , m_end(std::end(input))
    , m_line(line)
    , m_column(1) {}

  value::list::container_type
  parser::parse()
  {
    value::list::container_type values;

    for (;;)
    {
      skip_whitespace();
      if (eof())
      {
        return values;
      }
      values.push_back(parse_value());
    }
  }

  bool
  parser::eof() const
  {
    return m_pos >= m_end;
  }

  void
  parser::skip_whitespace()
  {
    while (!eof())
    {
      // Skip line comments.
      if (peek_read('#'))
      {
        while (!eof())
        {
          if (peek_read('\n') || peek_read('\r'))
          {
            break;
          } else {
            read();
          }
        }
      }
      else if (!peek_read(isspace))
      {
        return;
      }
    }
  }

  value::ptr
  parser::parse_value()
  {
    int line;
    int column;

    skip_whitespace();

    line = m_line;
    column = m_column;

    if (eof())
    {
      throw error("Unexpected end of input, missing token.", line, column);
    }

    if (peek_read('('))
    {
      value::list::container_type elements;

      for (;;)
      {
        skip_whitespace();

        if (eof())
        {
          throw error("Unterminated list: Missing `)'.", line, column);
        }
        else if (peek_read(')'))
        {
          break;
        }
        elements.push_back(parse_value());
      }

      return make_list(elements, line, column);
    }
    else if (peek_read('\''))
    {
      return make_list(
        {
          make_atom("quote", line, column),
          parse_value()
        },
        line,
        column
      );
    } else {
      std::string id;

      id += read();
      while (
        !eof() &&
        !std::isspace(*m_pos) &&
        *m_pos != '#' &&
        *m_pos != '(' &&
        *m_pos != ')' &&
        *m_pos != '\''
      )
      {
        id += read();
      }

      return make_atom(id, line, column);
    }
  }

  char
  parser::read()
  {
    const auto result = *m_pos++;

    if (result == '\n')
    {
      ++m_line;
      m_column = 1;
    } else {
      ++m_column;
    }

    return result;
  }

  bool
  parser::peek(char input) const
  {
    return !eof() && *m_pos == input;
  }

  bool
  parser::peek_read(char input)
  {
    if (peek(input))
    {
      read();

      return true;
    }

    return false;
  }

  bool
  parser::peek_read(bool (*callback)(char))
  {
    if (!eof() && callback(*m_pos))
    {
      read();

      return true;
    }

    return false;
  }
}
