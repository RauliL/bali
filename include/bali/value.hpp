#pragma once

#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace bali
{
  class value
  {
  public:
    using ptr = std::shared_ptr<value>;

    enum class type
    {
      atom,
      function,
      list,
    };

    class atom;
    class function;
    class list;

    static inline std::u32string to_string(const ptr& value)
    {
      return value ? value->to_string() : U"nil";
    }

    explicit value(
      const std::optional<int>& line = std::nullopt,
      const std::optional<int>& column = std::nullopt
    );
    value(const value&) = delete;
    value(value&&) = delete;
    void operator=(const value&) = delete;
    void operator=(value&&) = delete;

    virtual enum type type() const = 0;

    inline const std::optional<int>& line() const
    {
      return m_line;
    }

    inline const std::optional<int>& column() const
    {
      return m_column;
    }

  protected:
    virtual std::u32string to_string() const = 0;

  private:
    const std::optional<int> m_line;
    const std::optional<int> m_column;
  };

  class value::atom final : public value
  {
  public:
    using value_type = std::u32string;
    using const_reference = const value_type&;

    static inline std::shared_ptr<atom> make(
      const_reference symbol,
      const std::optional<int>& line = std::nullopt,
      const std::optional<int>& column = std::nullopt
    )
    {
      return std::shared_ptr<atom>(new atom(symbol, line, column));
    }

    static inline std::shared_ptr<atom> make_bool(
      bool value,
      const std::optional<int>& line = std::nullopt,
      const std::optional<int>& column = std::nullopt
    )
    {
      return value
        ? std::shared_ptr<atom>(new atom(U"true", line, column))
        : nullptr;
    }

    static std::shared_ptr<atom> make_number(
      double value,
      const std::optional<int>& line = std::nullopt,
      const std::optional<int>& column = std::nullopt
    );

    inline enum type type() const
    {
      return type::atom;
    }

    inline const_reference symbol() const
    {
      return m_symbol;
    }

  protected:
    inline std::u32string to_string() const
    {
      return m_symbol;
    }

  private:
    explicit atom(
      const_reference symbol,
      const std::optional<int>& line,
      const std::optional<int>& column
    );

  private:
    const value_type m_symbol;
  };

  class value::list final : public value
  {
  public:
    using value_type = ptr;
    using container_type = std::vector<value_type>;
    using size_type = container_type::size_type;
    using iterator = container_type::const_iterator;

    static inline std::shared_ptr<list> make(
      const container_type& elements,
      const std::optional<int>& line = std::nullopt,
      const std::optional<int>& column = std::nullopt
    )
    {
      return std::shared_ptr<list>(new list(elements, line, column));
    }

    inline enum type type() const
    {
      return type::list;
    }

    inline const container_type& elements() const
    {
      return m_elements;
    }

  protected:
    std::u32string to_string() const;

  private:
    explicit list(
      const container_type& elements,
      const std::optional<int>& line,
      const std::optional<int>& column
    );

  private:
    const container_type m_elements;
  };

  class value::function : public value
  {
  public:
    class builtin;
    class custom;

    inline enum type type() const
    {
      return type::function;
    }

    inline const std::optional<std::u32string>& name() const
    {
      return m_name;
    }

    virtual value::ptr call(
      const value::list::container_type& arguments,
      const std::shared_ptr<class scope>& scope
    ) const = 0;

  protected:
    explicit function(
      const std::optional<std::u32string>& name,
      const std::optional<int>& line,
      const std::optional<int>& column
    );

  private:
    const std::optional<std::u32string> m_name;
  };

  class value::function::builtin final : public value::function
  {
  public:
    using callback_type = value::ptr(*)(
      value::list::iterator&,
      const value::list::iterator&,
      const std::shared_ptr<class scope>&
    );

    static inline std::shared_ptr<builtin> make(
      callback_type callback,
      const std::u32string& name
    )
    {
      return std::shared_ptr<builtin>(new builtin(callback, name));
    }

    value::ptr call(
      const value::list::container_type& arguments,
      const std::shared_ptr<class scope>& scope
    ) const;

  protected:
    std::u32string to_string() const;

  private:
    builtin(callback_type callback, const std::u32string& name);

  private:
    const callback_type m_callback;
  };

  class value::function::custom final : public value::function
  {
  public:
    static inline std::shared_ptr<custom>
    make(
      const std::vector<std::u32string>& parameters,
      const ptr& expression,
      const std::optional<std::u32string>& name = std::nullopt,
      const std::optional<int>& line = std::nullopt,
      const std::optional<int>& column = std::nullopt
    )
    {
      return std::shared_ptr<custom>(new custom(
        parameters,
        expression,
        name,
        line,
        column
      ));
    }

    value::ptr call(
      const value::list::container_type& arguments,
      const std::shared_ptr<class scope>& scope
    ) const;

  protected:
    std::u32string to_string() const;

  private:
    explicit custom(
      const std::vector<std::u32string>& parameters,
      const ptr& expression,
      const std::optional<std::u32string>& name,
      const std::optional<int>& line,
      const std::optional<int>& column
    );

  private:
    const std::vector<std::u32string> m_parameters;
    const ptr m_expression;
  };

  std::ostream& operator<<(std::ostream& os, const value::ptr& value);
}
