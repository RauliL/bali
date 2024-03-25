#pragma once

#include <bali/scope.hpp>

namespace bali
{
  using builtin_function_callback_type = value::ptr(*)(
    value::list::iterator&,
    const value::list::iterator&,
    const std::shared_ptr<class scope>&
  );

  std::shared_ptr<value> call_function(
    const std::u32string& name,
    value::list::iterator& begin,
    const value::list::iterator& end,
    const std::shared_ptr<class scope>& scope,
    const std::optional<int>& line = std::nullopt,
    const std::optional<int>& column = std::nullopt
  );

  std::shared_ptr<value::function> define_custom_function(
    const std::u32string& name,
    const std::vector<std::u32string>& parameters,
    const value::ptr& expression
  );
}
