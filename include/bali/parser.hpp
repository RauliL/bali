#pragma once

#include <bali/value.hpp>

namespace bali
{
  value::list::container_type
  parse(const std::string& input, int line = 1);
}
