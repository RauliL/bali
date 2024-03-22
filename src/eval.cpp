#include <bali/error.hpp>
#include <bali/eval.hpp>
#include <bali/function.hpp>

namespace bali
{
  static value::ptr
  eval_atom(
    const std::shared_ptr<value::atom>& atom,
    const std::shared_ptr<class scope>& scope
  )
  {
    const auto id = atom->symbol();
    value::ptr variable;

    if (scope->get(id, variable))
    {
      return variable;
    }

    return !id.compare("nil") ? nullptr : atom;
  }

  static value::ptr
  eval_list(
    const std::shared_ptr<value::list>& list,
    const std::shared_ptr<class scope>& scope
  )
  {
    const auto& elements = list->elements();

    if (elements.size() > 0)
    {
      auto begin = std::begin(elements);
      const auto end = std::end(elements);
      const auto id = to_atom(*begin++, scope);

      if (const auto function = find_builtin_function(id))
      {
        return (*function)(begin, end, scope);
      }

      throw error(
        "Unrecognized function: `" + id + "'",
        list->line(),
        list->column()
      );
    }

    return list;
  }

  value::ptr
  eval(
    const value::ptr& value,
    const std::shared_ptr<class scope>& scope
  )
  {
    if (!value)
    {
      return nullptr;
    }

    switch (value->type())
    {
      case value::type::atom:
        return eval_atom(std::static_pointer_cast<value::atom>(value), scope);

      case value::type::list:
        return eval_list(std::static_pointer_cast<value::list>(value), scope);
    }

    return value;
  }
}
