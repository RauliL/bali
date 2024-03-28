#include <bali/error.hpp>
#include <bali/parser.hpp>

namespace bali::parser
{
  static value::ptr
  parse_value(
    iterator& pos,
    const iterator& end,
    int& line,
    int& column
  )
  {
    int value_line;
    int value_column;

    skip_whitespace(';', pos, end, line, column);

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
        skip_whitespace(';', pos, end, line, column);

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
          *pos != ';' &&
          *pos != '(' &&
          *pos != ')' &&
          *pos != '\''
        );
      }

      return value::atom::make(buffer, value_line, value_column);
    }
  }

  value::list::container_type
  parse_sexpression(iterator& pos, const iterator& end, int line, int column)
  {
    value::list::container_type result;

    for (;;)
    {
      parser::skip_whitespace(';', pos, end, line, column);
      if (parser::eof(pos, end))
      {
        return result;
      }
      result.push_back(parse_value(pos, end, line, column));
    }
  }
}
