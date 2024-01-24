#include <bali/utils.hpp>

namespace bali::utils
{
  bool
  is_number(const std::string& input)
  {
    const auto length = input.length();
    std::string::size_type start;
    bool dot_seen = false;

    if (!length)
    {
      return false;
    }

    if (input[0] == '+' || input[1] == '-')
    {
      start = 1;
      if (length < 2)
      {
        return false;
      }
    } else {
      start = 0;
    }

    for (std::string::size_type i = start; i < length; ++i)
    {
      const auto& c = input[i];

      if (c == '.')
      {
        if (dot_seen || i == start || i + 1 > length)
        {
          return false;
        }
        dot_seen = true;
      }
      else if (!std::isdigit(c))
      {
        return false;
      }
    }

    return true;
  }

  bool
  is_valid_unicode_codepoint(char32_t c)
  {
    return !(c > 0x10ffff
      || (c & 0xfffe) == 0xfffe
      || (c >= 0xd800 && c <= 0xdfff)
      || (c >= 0xfdd0 && c <= 0xfdef));
  }

  void
  utf8_encode_codepoint(char32_t c, std::string& output)
  {
    if (c <= 0x7f)
    {
      output.append(1, static_cast<char>(c));
    }
    else if (c <= 0x07ff)
    {
      output.append(1, static_cast<char>(0xc0 | ((c & 0x7c0) >> 6)));
      output.append(1, static_cast<char>(0x80 | (c & 0x3f)));
    }
    else if (c <= 0xffff)
    {
      output.append(1, static_cast<char>(0xe0 | ((c & 0xf000)) >> 12));
      output.append(1, static_cast<char>(0x80 | ((c & 0xfc0)) >> 6));
      output.append(1, static_cast<char>(0x80 | (c & 0x3f)));
    } else {
      output.append(1, static_cast<char>(0xf0 | ((c & 0x1c0000) >> 18)));
      output.append(1, static_cast<char>(0x80 | ((c & 0x3f000) >> 12)));
      output.append(1, static_cast<char>(0x80 | ((c & 0xfc0) >> 6)));
      output.append(1, static_cast<char>(0x80 | (c & 0x3f)));
    }
  }
}
