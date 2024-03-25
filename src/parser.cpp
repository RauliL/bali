#include <cctype>

#include <peelo/unicode/ctype/isvalid.hpp>
#include <peelo/unicode/encoding/utf8.hpp>

#include <bali/error.hpp>
#include <bali/parser.hpp>
#include <bali/utils.hpp>

namespace bali
{
  static const char32_t comment_character = U';';

  static inline bool
  eof(
    std::string::const_iterator& pos,
    const std::string::const_iterator& end
  )
  {
    return pos >= end;
  }

  static char32_t
  read(
    std::string::const_iterator& pos,
    const std::string::const_iterator& end,
    int& line,
    int& column
  )
  {
    using peelo::unicode::encoding::utf8::decode_advance;
    using peelo::unicode::encoding::utf8::sequence_length;
    const auto size = sequence_length(*pos);
    char32_t result;

    if (size > 1)
    {
      char buffer[size];
      std::size_t i = 0;
      char32_t result;

      if ((pos + (size - 1)) >= end)
      {
        throw error(U"Invalid UTF-8 sequence.", line, column);
      }
      for (std::size_t i = 0; i < size; ++i)
      {
        buffer[i] = *(pos + i);
      }
      pos += size;
      if (!decode_advance(buffer, i, size, result))
      {
        throw error(U"Invalid UTF-8 sequence.", line, column);
      }

      return result;
    }
    else if (size < 1)
    {
      throw error(U"Invalid UTF-8 sequence.", line, column);
    } else {
      result = static_cast<char32_t>(*pos++);
    }
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
      read(pos, end, line, column);

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
            read(pos, end, line, column);
          }
        }
      }
      else if (std::isspace(*pos))
      {
        read(pos, end, line, column);
      } else {
        return;
      }
    }
  }

  static void
  parse_escape_sequence(
    std::u32string& buffer,
    std::string::const_iterator& pos,
    const std::string::const_iterator& end,
    int& line,
    int& column
  )
  {
    using peelo::unicode::ctype::isvalid;
    const int sequence_line = line;
    const int sequence_column = column;

    if (eof(pos, end))
    {
      throw error(
        U"Unexpected end of input; Missing escape sequence.",
        sequence_line,
        sequence_column
      );
    }

    switch (read(pos, end, line, column))
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
              U"Unterminated escape sequence.",
              sequence_line,
              sequence_column
            );
          }
          else if (!std::isxdigit(*pos))
          {
            throw error(
              U"Illegal Unicode hex escape sequence.",
              sequence_line,
              sequence_column
            );
          }

          if (*pos >= 'A' && *pos <= 'F')
          {
            result = result * 16 + (read(pos, end, line, column) - 'A' + 10);
          }
          else if (*pos >= 'a' && *pos <= 'f')
          {
            result = result * 16 + (read(pos, end, line, column) - 'a' + 10);
          } else {
            result = result * 16 + (read(pos, end, line, column) - '0');
          }
        }

        if (!isvalid(result))
        {
          throw error(
            U"Illegal Unicode hex escape sequence.",
            sequence_line,
            sequence_column
          );
        }

        buffer += result;
      }
      break;

    default:
      throw error(
        U"Illegal escape sequence.",
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
        U"Unexpected end of input, missing token.",
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
            U"Unterminated list: Missing `)'.",
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
          value::atom::make(U"quote", value_line, value_column),
          parse_value(pos, end, line, column)
        },
        value_line,
        value_column
      );
    } else {
      std::u32string buffer;

      if (peek_read('"', pos, end, line, column))
      {
        for (;;)
        {
          if (eof(pos, end))
          {
            throw error(
              U"Unterminated string: Missing `\"'.",
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
            buffer += read(pos, end, line, column);
          }
        }
      } else {
        do
        {
          if (peek_read('\\', pos, end, line, column))
          {
            parse_escape_sequence(buffer, pos, end, line, column);
          } else {
            buffer += read(pos, end, line, column);
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
      }

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
        read(pos, end, line, column);
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
