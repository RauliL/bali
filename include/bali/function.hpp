#pragma once

#include <bali/value.hpp>

namespace bali
{
  using builtin_function_callback_type = value::ptr(*)(
    value::list::iterator&,
    const value::list::iterator&,
    const std::shared_ptr<class scope>&
  );

  std::optional<builtin_function_callback_type> find_builtin_function(
    const std::string& name
  );
  std::shared_ptr<value::function> find_custom_function(
    const std::string& name
  );
  std::shared_ptr<value::function> define_custom_function(
    const std::string& name,
    const std::vector<std::string>& parameters,
    const value::ptr& expression
  );
}
