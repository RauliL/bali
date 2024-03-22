#include <bali/scope.hpp>

namespace bali
{
  scope::scope(const std::shared_ptr<scope>& parent)
    : m_parent(parent) {}

  bool
  scope::has(const std::string& name) const
  {
    if (m_variables.find(name) != std::end(m_variables))
    {
      return true;
    }

    return m_parent && m_parent->has(name);
  }

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
    if (m_parent && m_parent->has(name))
    {
      m_parent->set(name, value);
    } else {
      m_variables[name] = value;
    }
  }
}
