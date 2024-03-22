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

  private:
    const std::optional<int> m_line;
    const std::optional<int> m_column;
  };

  class value::atom final : public value
  {
  public:
    explicit atom(
      const std::string& symbol,
      const std::optional<int>& line = std::nullopt,
      const std::optional<int>& column = std::nullopt
    );

    inline enum type type() const
    {
      return type::atom;
    }

    inline const std::string& symbol() const
    {
      return m_symbol;
    }

  private:
    const std::string m_symbol;
  };

  class value::list final : public value
  {
  public:
    using value_type = ptr;
    using container_type = std::vector<value_type>;
    using iterator = container_type::const_iterator;

    explicit list(
      const container_type& elements,
      const std::optional<int>& line = std::nullopt,
      const std::optional<int>& column = std::nullopt
    );

    inline enum type type() const
    {
      return type::list;
    }

    inline const container_type& elements() const
    {
      return m_elements;
    }

  private:
    const container_type m_elements;
  };

  class value::function final : public value
  {
  public:
    explicit function(
      const std::vector<std::string>& parameters,
      const ptr& expression,
      const std::optional<std::string>& name,
      const std::optional<int>& line = std::nullopt,
      const std::optional<int>& column = std::nullopt
    );

    inline enum type type() const
    {
      return type::function;
    }

    inline const std::optional<std::string>& name() const
    {
      return m_name;
    }

    value::ptr call(
      value::list::iterator& begin,
      const value::list::iterator& end,
      const std::shared_ptr<class scope>& scope
    ) const;

  private:
    const std::vector<std::string> m_parameters;
    const ptr m_expression;
    const std::optional<std::string> m_name;
  };

  std::string to_string(const value::ptr&);

  std::shared_ptr<value::atom> make_bool(
    bool value,
    const std::optional<int>& line = std::nullopt,
    const std::optional<int>& column = std::nullopt
  );
  std::shared_ptr<value::atom> make_number(
    double value,
    const std::optional<int>& line = std::nullopt,
    const std::optional<int>& column = std::nullopt
  );
  std::shared_ptr<value::atom> make_atom(
    const std::string& symbol,
    const std::optional<int>& line = std::nullopt,
    const std::optional<int>& column = std::nullopt
  );
  std::shared_ptr<value::list> make_list(
    const value::list::container_type& elements,
    const std::optional<int>& line = std::nullopt,
    const std::optional<int>& column = std::nullopt
  );
  std::shared_ptr<value::function> make_function(
    const std::vector<std::string>& parameters,
    const value::ptr& expression,
    const std::optional<std::string>& name = std::nullopt,
    const std::optional<int>& line = std::nullopt,
    const std::optional<int>& column = std::nullopt
  );

  std::ostream& operator<<(std::ostream& os, const value::ptr& value);
}
