#include <cctype>

#include <bali/error.hpp>
#include <bali/parser.hpp>

namespace bali
{
  static const char comment_character = ';';

  parser::parser(const std::string& input, int line)
    : m_pos(std::begin(input))
    , m_end(std::end(input))
    , m_line(line)
    , m_column(1) {}

  value::list::container_type
  parser::parse()
  {
    value::list::container_type values;

    // Skip shebang.
    if (m_pos + 1 != m_end && *m_pos == '#' && *(m_pos + 1) == '!')
    {
      for (;;)
      {
        if (eof() || peek_read('\n'))
        {
          break;
        }
        read();
      }
    }

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
      if (peek_read(comment_character))
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
      else if (std::isspace(*m_pos))
      {
        read();
      } else {
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

      return value::list::make(elements, line, column);
    }
    else if (peek_read('\''))
    {
      return value::list::make(
        {
          value::atom::make("quote", line, column),
          parse_value()
        },
        line,
        column
      );
    }
    else if (peek_read('"'))
    {
      std::string buffer;

      for (;;)
      {
        if (eof())
        {
          throw error("Unterminated string: Missing `\"'.", line, column);
        }
        else if (peek_read('"'))
        {
          break;
        }
        else if (peek_read('\\'))
        {
          parse_escape_sequence(buffer);
        } else {
          buffer += read();
        }
      }

      return value::atom::make(buffer, line, column);
    } else {
      std::string buffer;

      do
      {
        if (peek_read('\\'))
        {
          parse_escape_sequence(buffer);
        } else {
          buffer += read();
        }
      }
      while (
        !eof() &&
        !std::isspace(*m_pos) &&
        *m_pos != comment_character &&
        *m_pos != '(' &&
        *m_pos != ')' &&
        *m_pos != '\''
      );

      return value::atom::make(buffer, line, column);
    }
  }

  void
  parser::parse_escape_sequence(std::string& buffer)
  {
    const int line = m_line;
    const int column = m_column;

    if (eof())
    {
      throw error(
        "Unexpected end of input; Missing escape sequence.",
        line,
        column
      );
    }

    switch (read())
    {
    case 'b':
      buffer.append(1, 010);
      break;

    case 't':
      buffer.append(1, 011);
      break;

    case 'n':
      buffer.append(1, 012);
      break;

    case 'f':
      buffer.append(1, 014);
      break;

    case 'r':
      buffer.append(1, 015);
      break;

    case '"':
    case '\'':
    case '\\':
    case '/':
      buffer.append(1, *(m_pos - 1));
      break;

    case 'u':
      {
        char32_t result = 0;

        for (int i = 0; i < 4; ++i)
        {
          if (eof())
          {
            throw error(
              "Unterminated escape sequence.",
              line,
              column
            );
          }
          else if (!std::isxdigit(*m_pos))
          {
            throw error(
              "Illegal Unicode hex escape sequence.",
              line,
              column
            );
          }

          if (*m_pos >= 'A' && *m_pos <= 'F')
          {
            result = result * 16 + (read() - 'A' + 10);
          }
          else if (*m_pos >= 'a' && *m_pos <= 'f')
          {
            result = result * 16 + (read() - 'a' + 10);
          } else {
            result = result * 16 + (read() - '0');
          }
        }

        if (!utils::is_valid_unicode_codepoint(result))
        {
          throw error(
            "Illegal Unicode hex escape sequence.",
            line,
            column
          );
        }

        utils::utf8_encode_codepoint(result, buffer);
      }
      break;

    default:
      throw error(
        "Illegal escape sequence.",
        line,
        column
      );
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
  parser::peek_read(char input)
  {
    if (!eof() && *m_pos == input)
    {
      read();

      return true;
    }

    return false;
  }
}
