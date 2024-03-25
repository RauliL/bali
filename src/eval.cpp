#include <peelo/unicode/encoding/utf8.hpp>

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

    return !id.compare(U"nil") ? nullptr : atom;
  }

  static value::ptr
  eval_list(
    const std::shared_ptr<value::list>& list,
    const std::shared_ptr<class scope>& scope
  )
  {
    const auto& elements = list->elements();
    const auto size = elements.size();

    if (size > 0)
    {
      const auto id = to_atom(elements[0], scope);
      auto begin = std::begin(elements) + 1;
      const auto end = std::end(elements);

      return call_function(
        id,
        begin,
        end,
        scope,
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

  const std::u32string&
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
      U"Value is not an atom.",
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

    switch (result->type())
    {
      case value::type::atom:
        return std::static_pointer_cast<value::atom>(
          result
        )->symbol().compare(U"nil") != 0;

      case value::type::list:
        return std::static_pointer_cast<value::list>(
          result
        )->elements().size() > 0;

      default:
        return true;
    }
  }

  std::shared_ptr<value::function>
  to_function(
    const value::ptr& value,
    const std::shared_ptr<class scope>& scope
  )
  {
    const auto result = scope ? eval(value, scope) : value;

    if (result && result->type() == value::type::function)
    {
      return std::static_pointer_cast<value::function>(result);
    }

    throw error(
      U"Value is not a function.",
      value ? value->line() : std::nullopt,
      value ? value->column() : std::nullopt
    );
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
      U"Value is not a list.",
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
        return std::stod(peelo::unicode::encoding::utf8::encode(symbol));
      }
    }

    throw error(
      U"Value is not a number.",
      value ? value->line() : std::nullopt,
      value ? value->column() : std::nullopt
    );
  }
}
