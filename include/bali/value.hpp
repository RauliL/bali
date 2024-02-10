#pragma once

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
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
      list,
    };

    class atom;
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

  value::ptr eval(
    const value::ptr& value,
    std::unordered_map<std::string, value::ptr>& scope
  );
  std::string to_atom(
    const value::ptr& value,
    std::unordered_map<std::string, value::ptr>& scope
  );
  bool to_bool(
    const value::ptr& value,
    std::unordered_map<std::string, value::ptr>& scope
  );
  value::list::container_type to_list(
    const value::ptr& value,
    std::unordered_map<std::string, value::ptr>& scope
  );
  double to_number(
    const value::ptr& value,
    std::unordered_map<std::string, value::ptr>& scope
  );
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

  std::ostream& operator<<(std::ostream& os, const value::ptr& value);
}