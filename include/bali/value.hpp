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

    static inline std::string to_string(const ptr& value)
    {
      return value ? value->to_string() : "nil";
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
    virtual std::string to_string() const = 0;

  private:
    const std::optional<int> m_line;
    const std::optional<int> m_column;
  };

  class value::atom final : public value
  {
  public:
    using value_type = std::string;
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
        ? std::shared_ptr<atom>(new atom("true", line, column))
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
    inline std::string to_string() const
    {
      return m_symbol;
    }

  private:
    explicit atom(
      const_reference symbol,
      const std::optional<int>& line = std::nullopt,
      const std::optional<int>& column = std::nullopt
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
    std::string to_string() const;

  private:
    explicit list(
      const container_type& elements,
      const std::optional<int>& line = std::nullopt,
      const std::optional<int>& column = std::nullopt
    );

  private:
    const container_type m_elements;
  };

  class value::function final : public value
  {
  public:
    static inline std::shared_ptr<function>
    make(
      const std::vector<std::string>& parameters,
      const ptr& expression,
      const std::optional<std::string>& name = std::nullopt,
      const std::optional<int>& line = std::nullopt,
      const std::optional<int>& column = std::nullopt
    )
    {
      return std::shared_ptr<function>(new function(
        parameters,
        expression,
        name,
        line,
        column
      ));
    }

    inline enum type type() const
    {
      return type::function;
    }

    inline const std::optional<std::string>& name() const
    {
      return m_name;
    }

    value::ptr call(
      const value::list::container_type& arguments,
      const std::shared_ptr<class scope>& scope
    ) const;

  protected:
    std::string to_string() const;

  private:
    explicit function(
      const std::vector<std::string>& parameters,
      const ptr& expression,
      const std::optional<std::string>& name,
      const std::optional<int>& line = std::nullopt,
      const std::optional<int>& column = std::nullopt
    );

  private:
    const std::vector<std::string> m_parameters;
    const ptr m_expression;
    const std::optional<std::string> m_name;
  };

  std::ostream& operator<<(std::ostream& os, const value::ptr& value);
}
