#pragma once

#include <string>

namespace bali::utils
{
  bool is_number(const std::string& input);
  bool is_valid_unicode_codepoint(char32_t c);
  void utf8_encode_codepoint(char32_t c, std::string& output);
}
