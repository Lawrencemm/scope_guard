/*
 *  Created on: 13/02/2018
 *      Author: ricab
 */

#ifndef SCOPE_GUARD_HPP_
#define SCOPE_GUARD_HPP_

#include <type_traits>
#include <functional>
#include <utility>

////////////////////////////////////////////////////////////////////////////////
namespace sg
{
  /**
   * RAII class to call a resetter function when leaving scope. The resetter
   * needs to be compatible with std::function<void()>.
   */
  template<typename Callback>
  class scope_guard // TODO put into detail namespace
  {
  public:
    template<typename = typename std::enable_if<
      std::is_constructible<std::function<void()>, Callback>::value>::type>
    explicit scope_guard(Callback&& callback);

    scope_guard(scope_guard&& other);
    ~scope_guard();

  private:
    Callback m_callback;
    bool m_active;

  };

  /// helper to create scope_guard and deduce template params
  template<typename Callback>
  scope_guard<Callback> make_scope_guard(Callback&& resetter);
}

////////////////////////////////////////////////////////////////////////////////
template<typename Callback>
template<typename>
sg::scope_guard<Callback>::scope_guard(Callback&& callback)
  : m_callback{std::forward<Callback>(callback)}
  , m_active{true}
{}

////////////////////////////////////////////////////////////////////////////////
template<typename Callback>
sg::scope_guard<Callback>::~scope_guard()
{
  if(m_active)
    m_callback();
}

////////////////////////////////////////////////////////////////////////////////
template<typename Callback>
sg::scope_guard<Callback>::scope_guard(scope_guard&& other)
  : m_callback{std::forward<Callback>(other.m_callback)}
  , m_active{std::move(other.m_active)}
{
  other.m_active = false;
}

////////////////////////////////////////////////////////////////////////////////
template<typename Callback>
inline auto sg::make_scope_guard(Callback&& resetter) -> scope_guard<Callback>
{
  return scope_guard<Callback>{std::forward<Callback>(resetter)};
}

#endif /* SCOPE_GUARD_HPP_ */
