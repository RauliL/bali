#include <bali/utils.hpp>

namespace bali::utils
{
  bool
  is_number(const std::u32string& input)
  {
    const auto length = input.length();
    std::u32string::size_type start;
    bool dot_seen = false;

    if (!length)
    {
      return false;
    }

    if (input[0] == U'+' || input[1] == U'-')
    {
      start = 1;
      if (length < 2)
      {
        return false;
      }
    } else {
      start = 0;
    }

    for (std::u32string::size_type i = start; i < length; ++i)
    {
      const auto& c = input[i];

      if (c == U'.')
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
}
