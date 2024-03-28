#include <unordered_map>

#include <bali/error.hpp>
#include <bali/parser.hpp>

// TODO:
// - Lambda shortcut.
// - Cond shortcut.
//
// Some links to learn more about M-expressions:
// https://github.com/eignnx/misp
// https://medium.com/@kenichisasagawa/m-expressions-and-the-geek-transformation-a-nostalgic-tale-0b99d34ef38f
// http://informatimago.free.fr/i/develop/lisp/com/informatimago/small-cl-pgms/m-expression/m-expression.lisp

namespace bali::parser
{
  namespace
  {
    struct token
    {
      enum class type
      {
        atom,
        lparen,
        rparen,
        lbracket,
        rbracket,
        colon,
        semicolon,
        arrow,
      };

      enum type type;
      std::optional<std::u32string> symbol;
      int line;
      int column;
    };
  }

  using token_iterator = std::vector<token>::const_iterator;
  using operator_callback_type = value::ptr(*)(
    token_iterator&,
    const token_iterator&
  );

  static const std::unordered_map<char, enum token::type> separator_map =
  {
    { '(', token::type::lparen },
    { ')', token::type::rparen },
    { '[', token::type::lbracket },
    { ']', token::type::rbracket },
    { ',', token::type::colon },
    { ';', token::type::semicolon },
  };

  static inline value::ptr
  parse_expression(token_iterator& pos, const token_iterator& end);

  static std::u32string
  to_string(enum token::type type)
  {
    switch (type)
    {
      case token::type::atom:
        return U"atom";

      case token::type::lparen:
        return U"`('";

      case token::type::rparen:
        return U"`)'";

      case token::type::lbracket:
        return U"`['";

      case token::type::rbracket:
        return U"`]'";

      case token::type::colon:
        return U"`,'";

      case token::type::semicolon:
        return U"`;'";

      case token::type::arrow:
        return U"`->'";
    }

    return U"unknown token";
  }

  static inline bool
  is_symbol(char32_t c)
  {
    return separator_map.find(c) == std::end(separator_map) &&
      c != '#' &&
      c != '+' &&
      c != '-' &&
      c != '*' &&
      c != '/' &&
      c != '=' &&
      c != '"';
  }

  static std::vector<token>
  tokenize(iterator& pos, const iterator& end, int& line, int& column)
  {
    std::vector<token> result;

    for (;;)
    {
      int token_line;
      int token_column;

      skip_whitespace('#', pos, end, line, column);
      if (eof(pos, end))
      {
        break;
      }
      token_line = line;
      token_column = column;
      if (auto it = separator_map.find(*pos); it != std::end(separator_map))
      {
        read(pos, end, line, column);
        result.push_back({
          it->second,
          std::nullopt,
          token_line,
          token_column
        });
      }
      else if (peek_read('<', pos, end, line, column))
      {
        result.push_back({
          token::type::atom,
          peek_read('=', pos, end, line, column) ? U"<=" : U"<",
          token_line,
          token_column
        });
      }
      else if (peek_read('>', pos, end, line, column))
      {
        result.push_back({
          token::type::atom,
          peek_read('=', pos, end, line, column) ? U">=" : U">",
          token_line,
          token_column
        });
      }
      else if (peek_read('-', pos, end, line, column))
      {
        if (peek_read('>', pos, end, line, column))
        {
          result.push_back({
            token::type::arrow,
            std::nullopt,
            token_line,
            token_column
          });
        } else {
          result.push_back({
            token::type::atom,
            U"-",
            token_line,
            token_column
          });
        }
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
                token_line,
                token_column
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
          while (!eof(pos, end) && !std::isspace(*pos) && is_symbol(*pos));
        }
        result.push_back({
          token::type::atom,
          buffer,
          token_line,
          token_column
        });
      }
    }

    return result;
  }

  static inline bool
  peek_token(
    enum token::type type,
    token_iterator& pos,
    const token_iterator& end
  )
  {
    return pos < end && pos->type == type;
  }

  static inline bool
  peek_read_token(
    enum token::type type,
    token_iterator& pos,
    const token_iterator& end
  )
  {
    if (peek_token(type, pos, end))
    {
      ++pos;

      return true;
    }

    return false;
  }

  static inline bool
  peek_atom(
    const std::u32string& symbol,
    token_iterator& pos,
    const token_iterator& end
  )
  {
    return peek_token(token::type::atom, pos, end) &&
      !pos->symbol->compare(symbol);
  }

  static inline bool
  peek_read_atom(
    const std::u32string& symbol,
    token_iterator& pos,
    const token_iterator& end
  )
  {
    if (peek_atom(symbol, pos, end))
    {
      ++pos;

      return true;
    }

    return false;
  }

  static value::list::container_type
  parse_list(token_iterator& pos, const token_iterator& end)
  {
    const auto line = pos->line;
    const auto column = pos->column;
    value::list::container_type result;

    ++pos;
    if (peek_read_token(token::type::rbracket, pos, end))
    {
      return result;
    }
    for (;;)
    {
      result.push_back(parse_expression(pos, end));
      if (peek_read_token(token::type::rbracket, pos, end))
      {
        return result;
      }
      else if (peek_read_token(token::type::semicolon, pos, end))
      {
        continue;
      }

      throw error(U"Unterminated list, missing `]'.", line, column);
    }
  }

  static value::ptr
  parse_primary(token_iterator& pos, const token_iterator& end)
  {
    if (pos >= end)
    {
      throw error(U"Unexpected end of input, missing expression.");
    }

    const auto line = pos->line;
    const auto column = pos->column;

    if (peek_read_token(token::type::lparen, pos, end))
    {
      value::list::container_type elements;

      if (!peek_read_token(token::type::rparen, pos, end))
      {
        for (;;)
        {
          elements.push_back(parse_expression(pos, end));
          if (peek_read_token(token::type::rparen, pos, end))
          {
            break;
          }
          else if (peek_read_token(token::type::colon, pos, end))
          {
            continue;
          }

          throw error(U"Unterminated list, missing `)'.", line, column);
        }
      }

      return value::list::make(
        {
          value::atom::make(U"quote", line, column),
          value::list::make(elements, line, column)
        },
        line,
        column
      );
    }

    if (peek_token(token::type::lbracket, pos, end))
    {
      return value::list::make(
        {
          value::atom::make(U"quote", line, column),
          value::list::make(parse_list(pos, end), line, column)
        },
        line,
        column
      );
    }

    if (peek_token(token::type::atom, pos, end))
    {
      const auto atom = value::atom::make(*(pos++)->symbol, line, column);

      if (peek_token(token::type::lbracket, pos, end))
      {
        auto arguments = parse_list(pos, end);

        if (peek_read_atom(U"<=", pos, end))
        {
          return value::list::make(
            {
              value::atom::make(U"defun", line, column),
              atom,
              value::list::make(arguments, line, column),
              parse_expression(pos, end)
            },
            line,
            column
          );
        }
        arguments.insert(
          std::begin(arguments),
          1,
          value::list::make(
            { value::atom::make(U"quote", line, column), atom },
            line,
            column
          )
        );

        return value::list::make(arguments, line, column);
      }

      return atom;
    }

    throw error(
      U"Unexpected " + to_string(pos->type) + U", missing expression.",
      line,
      column
    );
  }

  static value::ptr
  parse_operator(
    token_iterator& pos,
    const token_iterator& end,
    operator_callback_type callback,
    const char32_t** operators
  )
  {
    auto expression = callback(pos, end);

AGAIN:
    for (std::size_t i = 0; operators[i]; ++i)
    {
      if (peek_atom(operators[i], pos, end))
      {
        const auto atom = value::atom::make(
          *pos->symbol,
          pos->line,
          pos->column
        );

        ++pos;
        expression = value::list::make(
          {
            atom,
            expression,
            callback(pos, end)
          },
          expression->line(),
          expression->column()
        );
        goto AGAIN;
      }
    }

    return expression;
  }

  static inline value::ptr
  parse_multiplicative(token_iterator& pos, const token_iterator& end)
  {
    static const char32_t* operators[3] = { U"*", U"/", nullptr };

    return parse_operator(pos, end, parse_primary, operators);
  }

  static inline value::ptr
  parse_additive(token_iterator& pos, const token_iterator& end)
  {
    static const char32_t* operators[3] = { U"+", U"-", nullptr };

    return parse_operator(pos, end, parse_multiplicative, operators);
  }

  static inline value::ptr
  parse_relational(token_iterator& pos, const token_iterator& end)
  {
    static const char32_t* operators[5] =
    {
      U"<",
      U">",
      U"<=",
      U">=",
      nullptr
    };

    return parse_operator(pos, end, parse_additive, operators);
  }

  static inline value::ptr
  parse_expression(token_iterator& pos, const token_iterator& end)
  {
    static const char32_t* operators[2] = { U"=", nullptr };

    return parse_operator(pos, end, parse_relational, operators);
  }

  value::list::container_type
  parse_mexpression(iterator& pos, const iterator& end, int line, int column)
  {
    const auto tokens = tokenize(pos, end, line, column);
    value::list::container_type result;
    auto token_pos = std::begin(tokens);
    const auto token_end = std::end(tokens);

    while (token_pos < token_end)
    {
      result.push_back(parse_expression(token_pos, token_end));
    }

    return result;
  }
}
