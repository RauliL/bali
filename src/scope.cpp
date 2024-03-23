#include <bali/scope.hpp>

namespace bali
{
  scope::scope(const std::shared_ptr<scope>& parent)
    : m_parent(parent) {}

  bool
  scope::get(const std::string& name, value::ptr& slot) const
  {
    const auto it = m_variables.find(name);

    if (it != std::end(m_variables))
    {
      slot = it->second;

      return true;
    }
    else if (m_parent)
    {
      return m_parent->get(name, slot);
    }

    return false;
  }

  void
  scope::let(const std::string& name, const value::ptr& value)
  {
    m_variables[name] = value;
  }

  void
  scope::set(const std::string& name, const value::ptr& value)
  {
    auto parent = m_parent;

    while (parent)
    {
      if (parent->m_variables.find(name) != std::end(parent->m_variables))
      {
        parent->m_variables[name] = value;
        return;
      }
      parent = parent->m_parent;
    }
    m_variables[name] = value;
  }
}
