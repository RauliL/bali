#pragma once

#include <unordered_map>

#include <bali/value.hpp>

namespace bali
{
  class scope
  {
  public:
    using container_type = std::unordered_map<std::u32string, value::ptr>;

    static std::shared_ptr<scope> make_top_level();

    explicit scope(const std::shared_ptr<scope>& parent = nullptr);
    scope(const scope&) = default;
    scope(scope&&) = default;
    scope& operator=(const scope&) = default;
    scope& operator=(scope&&) = default;

    bool get(const std::u32string& name, value::ptr& slot) const;
    void let(const std::u32string& name, const value::ptr& value);
    void set(const std::u32string& name, const value::ptr& value);

  private:
    std::shared_ptr<scope> m_parent;
    container_type m_variables;
  };
}
