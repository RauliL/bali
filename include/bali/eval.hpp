#pragma once

#include <bali/scope.hpp>

namespace bali
{
  value::ptr eval(
    const value::ptr& value,
    const std::shared_ptr<class scope>& scope
  );

  const std::u32string&
  to_atom(
    const value::ptr& value,
    const std::shared_ptr<class scope>& scope
  );

  bool
  to_bool(
    const value::ptr& value,
    const std::shared_ptr<class scope>& scope
  );

  std::shared_ptr<value::function>
  to_function(
    const value::ptr& value,
    const std::shared_ptr<class scope>& scope
  );

  const value::list::container_type&
  to_list(
    const value::ptr& value,
    const std::shared_ptr<class scope>& scope
  );

  double
  to_number(
    const value::ptr& value,
    const std::shared_ptr<class scope>& scope
  );
}
