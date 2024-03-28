#pragma once

#include <bali/value.hpp>

namespace bali
{
  namespace parser
  {
    using iterator = std::string::const_iterator;

    inline bool
    eof(iterator& pos, const iterator& end)
    {
      return pos >= end;
    }

    char32_t
    read(
      iterator& pos,
      const iterator& end,
      int& line,
      int& column
    );

    bool
    peek_read(
      char input,
      iterator& pos,
      const iterator& end,
      int& line,
      int& column
    );

    void
    skip_whitespace(
      char comment_character,
      iterator& pos,
      const iterator& end,
      int& line,
      int& column
    );

    void
    parse_escape_sequence(
      std::u32string& buffer,
      iterator& pos,
      const iterator& end,
      int& line,
      int& column
    );
  }

  value::list::container_type
  parse(
    const std::string& input,
    int line,
    int column,
    bool use_mexpression
  );
}
