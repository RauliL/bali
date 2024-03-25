#include <bali/scope.hpp>

namespace bali
{
  scope::scope(const std::shared_ptr<scope>& parent)
    : m_parent(parent) {}

  bool
  scope::get(const std::u32string& name, value::ptr& slot) const
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
  scope::let(const std::u32string& name, const value::ptr& value)
  {
    m_variables[name] = value;
  }

  void
  scope::set(const std::u32string& name, const value::ptr& value)
  {
    if (m_variables.find(name) != std::end(m_variables))
    {
      m_variables[name] = value;
      return;
    }

    for (auto parent = m_parent; parent; parent = parent->m_parent)
    {
      if (parent->m_variables.find(name) != std::end(parent->m_variables))
      {
        parent->m_variables[name] = value;
        return;
      }
    }

    m_variables[name] = value;
  }
}
