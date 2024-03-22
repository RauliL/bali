#pragma once

#include <bali/scope.hpp>

namespace bali
{
  value::ptr eval(
    const value::ptr& value,
    const std::shared_ptr<class scope>& scope
  );
}
