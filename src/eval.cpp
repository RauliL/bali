#include <bali/error.hpp>
#include <bali/eval.hpp>
#include <bali/function.hpp>
#include <bali/utils.hpp>

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

      if (const auto function = find_custom_function(id))
      {
        return function->call(begin, end, scope);
      }
      else if (const auto function = find_builtin_function(id))
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

      case value::type::function:
        break;
    }

    return value;
  }

  const std::string&
  to_atom(
    const value::ptr& value,
    const std::shared_ptr<class scope>& scope
  )
  {
    const auto result = scope ? eval(value, scope) : value;

    if (result && result->type() == value::type::atom)
    {
      return std::static_pointer_cast<value::atom>(result)->symbol();
    }

    throw error(
      "Value is not an atom.",
      value ? value->line() : std::nullopt,
      value ? value->column() : std::nullopt
    );
  }

  bool
  to_bool(
    const value::ptr& value,
    const std::shared_ptr<class scope>& scope
  )
  {
    const auto result = scope ? eval(value, scope) : value;

    if (!result)
    {
      return false;
    }
    else if (result->type() == value::type::atom)
    {
      return std::static_pointer_cast<value::atom>(
        result
      )->symbol().compare("nil") != 0;
    }

    return true;
  }

  const value::list::container_type&
  to_list(
    const value::ptr& value,
    const std::shared_ptr<class scope>& scope
  )
  {
    const auto result = scope ? eval(value, scope) : value;

    if (result && result->type() == value::type::list)
    {
      return std::static_pointer_cast<value::list>(result)->elements();
    }

    throw error(
      "Value is not a list.",
      value ? value->line() : std::nullopt,
      value ? value->column() : std::nullopt
    );
  }

  double
  to_number(
    const value::ptr& value,
    const std::shared_ptr<class scope>& scope
  )
  {
    const auto result = scope ? eval(value, scope) : value;

    if (result && result->type() == value::type::atom)
    {
      const auto& symbol = std::static_pointer_cast<value::atom>(
        result
      )->symbol();

      if (utils::is_number(symbol))
      {
        return std::stod(symbol);
      }
    }

    throw error(
      "Value is not a number.",
      value ? value->line() : std::nullopt,
      value ? value->column() : std::nullopt
    );
  }
}
