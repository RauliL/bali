#include <cctype>

#include <bali/error.hpp>
#include <bali/parser.hpp>
#include <bali/utils.hpp>

namespace bali
{
  static const char comment_character = ';';

  static inline bool
  eof(
    std::string::const_iterator& pos,
    const std::string::const_iterator& end
  )
  {
    return pos >= end;
  }

  static inline char
  read(
    std::string::const_iterator& pos,
    int& line,
    int& column
  )
  {
    const auto result = *pos++;

    if (result == '\n')
    {
      ++line;
      column = 1;
    } else {
      ++column;
    }

    return result;
  }

  static inline bool
  peek_read(
    char input,
    std::string::const_iterator& pos,
    const std::string::const_iterator& end,
    int& line,
    int& column
  )
  {
    if (!eof(pos, end) && *pos == input)
    {
      read(pos, line, column);

      return true;
    }

    return false;
  }

  static void
  skip_whitespace(
    std::string::const_iterator& pos,
    const std::string::const_iterator& end,
    int& line,
    int& column
  )
  {
    while (!eof(pos, end))
    {
      // Skip line comments.
      if (peek_read(comment_character, pos, end, line, column))
      {
        while (!eof(pos, end))
        {
          if (
            peek_read('\n', pos, end, line, column) ||
            peek_read('\r', pos, end, line, column)
          )
          {
            break;
          } else {
            read(pos, line, column);
          }
        }
      }
      else if (std::isspace(*pos))
      {
        read(pos, line, column);
      } else {
        return;
      }
    }
  }

  static void
  parse_escape_sequence(
    std::string& buffer,
    std::string::const_iterator& pos,
    const std::string::const_iterator& end,
    int& line,
    int& column
  )
  {
    const int sequence_line = line;
    const int sequence_column = column;

    if (eof(pos, end))
    {
      throw error(
        "Unexpected end of input; Missing escape sequence.",
        sequence_line,
        sequence_column
      );
    }

    switch (read(pos, line, column))
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
      buffer.append(1, *(pos - 1));
      break;

    case 'u':
      {
        char32_t result = 0;

        for (int i = 0; i < 4; ++i)
        {
          if (eof(pos, end))
          {
            throw error(
              "Unterminated escape sequence.",
              sequence_line,
              sequence_column
            );
          }
          else if (!std::isxdigit(*pos))
          {
            throw error(
              "Illegal Unicode hex escape sequence.",
              sequence_line,
              sequence_column
            );
          }

          if (*pos >= 'A' && *pos <= 'F')
          {
            result = result * 16 + (read(pos, line, column) - 'A' + 10);
          }
          else if (*pos >= 'a' && *pos <= 'f')
          {
            result = result * 16 + (read(pos, line, column) - 'a' + 10);
          } else {
            result = result * 16 + (read(pos, line, column) - '0');
          }
        }

        if (!utils::is_valid_unicode_codepoint(result))
        {
          throw error(
            "Illegal Unicode hex escape sequence.",
            sequence_line,
            sequence_column
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

  static value::ptr
  parse_value(
    std::string::const_iterator& pos,
    const std::string::const_iterator& end,
    int& line,
    int& column
  )
  {
    int value_line;
    int value_column;

    skip_whitespace(pos, end, line, column);

    value_line = line;
    value_column = column;

    if (eof(pos, end))
    {
      throw error(
        "Unexpected end of input, missing token.",
        value_line,
        value_column
      );
    }

    if (peek_read('(', pos, end, line, column))
    {
      value::list::container_type elements;

      for (;;)
      {
        skip_whitespace(pos, end, line, column);

        if (eof(pos, end))
        {
          throw error(
            "Unterminated list: Missing `)'.",
            value_line,
            value_column
          );
        }
        else if (peek_read(')', pos, end, line, column))
        {
          break;
        }
        elements.push_back(parse_value(pos, end, line, column));
      }

      return value::list::make(elements, value_line, value_column);
    }
    else if (peek_read('\'', pos, end, line, column))
    {
      return value::list::make(
        {
          value::atom::make("quote", value_line, value_column),
          parse_value(pos, end, line, column)
        },
        value_line,
        value_column
      );
    }
    else if (peek_read('"', pos, end, line, column))
    {
      std::string buffer;

      for (;;)
      {
        if (eof(pos, end))
        {
          throw error(
            "Unterminated string: Missing `\"'.",
            value_line,
            value_column
          );
        }
        else if (peek_read('"', pos, end, line, column))
        {
          break;
        }
        else if (peek_read('\\', pos, end, line, column))
        {
          parse_escape_sequence(buffer, pos, end, line, column);
        } else {
          buffer += read(pos, line, column);
        }
      }

      return value::atom::make(buffer, value_line, value_column);
    } else {
      std::string buffer;

      do
      {
        if (peek_read('\\', pos, end, line, column))
        {
          parse_escape_sequence(buffer, pos, end, line, column);
        } else {
          buffer += read(pos, line, column);
        }
      }
      while (
        !eof(pos, end) &&
        !std::isspace(*pos) &&
        *pos != comment_character &&
        *pos != '(' &&
        *pos != ')' &&
        *pos != '\''
      );

      return value::atom::make(buffer, value_line, value_column);
    }
  }

  value::list::container_type
  parse(const std::string& input, int line)
  {
    auto pos = std::begin(input);
    const auto end = std::end(input);
    int column = 1;
    value::list::container_type values;

    // Skip shebang.
    if (pos + 1 != end && *pos == '#' && *(pos + 1) == '!')
    {
      for (;;)
      {
        if (eof(pos, end) || peek_read('\n', pos, end, line, column))
        {
          break;
        }
        read(pos, line, column);
      }
    }

    for (;;)
    {
      skip_whitespace(pos, end, line, column);
      if (eof(pos, end))
      {
        return values;
      }
      values.push_back(parse_value(pos, end, line, column));
    }
  }
}
