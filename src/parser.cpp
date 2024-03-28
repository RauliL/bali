#include <cctype>

#include <peelo/unicode/ctype/isvalid.hpp>
#include <peelo/unicode/encoding/utf8.hpp>

#include <bali/error.hpp>
#include <bali/parser.hpp>

namespace bali
{
  namespace parser
  {
    char32_t
    read(
      iterator& pos,
      const iterator& end,
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

    bool
    peek_read(
      char input,
      iterator& pos,
      const iterator& end,
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

    void
    skip_whitespace(
      char comment_character,
      iterator& pos,
      const iterator& end,
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

    void
    parse_escape_sequence(
      std::u32string& buffer,
      iterator& pos,
      const iterator& end,
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

    static inline void
    skip_shebang(iterator& pos, const iterator& end, int& line, int& column)
    {
      if (pos + 1 != end && *pos == '#' && *(pos + 1) == '!')
      {
        for (;;)
        {
          if (
            parser::eof(pos, end) ||
            parser::peek_read('\n', pos, end, line, column)
          )
          {
            break;
          }
          parser::read(pos, end, line, column);
        }
      }
    }

    value::list::container_type
    parse_sexpression(
      iterator& pos,
      const iterator& end,
      int line,
      int column
    );

    value::list::container_type
    parse_mexpression(
      iterator& pos,
      const iterator& end,
      int line,
      int column
    );
  }

  value::list::container_type
  parse(const std::string& input, int line, int column, bool use_mexpression)
  {
    auto pos = std::begin(input);
    const auto end = std::end(input);

    parser::skip_shebang(pos, end, line, column);

    return use_mexpression
      ? parser::parse_mexpression(pos, end, line, column)
      : parser::parse_sexpression(pos, end, line, column);
  }
}
