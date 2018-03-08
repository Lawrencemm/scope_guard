/*
 *  Created on: 13/02/2018
 *      Author: ricab
 */

#include "scope_guard.hpp"

#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main()
#include "catch/catch.hpp"

using namespace sg;

// TODO test move guard into maker
// TODO add tests to show move assignment is forbidden
// TODO add test moved guard has no effect
// TODO add test to show function can still be called multiple times outside scope guard
// TODO add custom functor tests
// TODO add const functor test
// TODO add member function tests
// TODO add boost tests on conditional boost include finding
// TODO add actual rollback test
// TODO add temporary test
// TODO add new/delete tests
// TODO add unique_ptr tests
// TODO add shared_ptr tests
// TODO add move guard into function tests
// TODO add move guard into container tests
// TODO add tests for descending guard
// TODO add tests for no implicitly ignored return (and btw, make sure it would be implicitly ignored)
// TODO for bonus, support function overloads (not sure how or if at all possible)
// TODO add doxygen file and check correct documentation


////////////////////////////////////////////////////////////////////////////////
namespace
{
  auto count = 0u;
  void incc(unsigned& c) noexcept { ++c; }
  void inc() noexcept { incc(count); }
  void resetc(unsigned& c) noexcept { c = 0u; }
  void reset() noexcept { resetc(count); }

  template<typename Fun>
  struct remove_noexcept
  {
    using type = Fun;
  };

  template<typename Ret, typename... Args>
  struct remove_noexcept<Ret(Args...) noexcept(true)>
  {
    using type = Ret(Args...);
  };

  template<typename Fun>
  using remove_noexcept_t = typename remove_noexcept<Fun>::type;

  template<typename Fun>
  std::function<remove_noexcept_t<Fun>>
  make_std_function(Fun& f) // ref prevents decay to pointer (no move needed)
  {
    return std::function<
      remove_noexcept_t<typename std::remove_reference<Fun>::type>>{f}; /*
    unfortunately in C++17 std::function does not accept a noexcept target type
    (results in incomplete type - at least in gcc and clang) */
  }
}

////////////////////////////////////////////////////////////////////////////////
TEST_CASE("A plain function can be used to create a scope_guard.")
{
  make_scope_guard(inc);
}

////////////////////////////////////////////////////////////////////////////////
TEST_CASE("Demonstration that direct constructor call is possible, but not "
          "advisable.")
{
  using detail::scope_guard;
//  scope_guard{inc}; // Error: does not deduce template args (at least
                      // until C++17); it also does not accept everything...

//  scope_guard<decltype(inc)>{inc}; // ... Error: cannot instantiate data field
                                     // with function type...

  scope_guard<void(&)()noexcept>{
      static_cast<void(&)()noexcept>(inc)}; // ... could use ref cast...

  auto& inc_ref = inc;
  scope_guard<decltype(inc_ref)>{inc_ref}; // ... or actual ref...

  scope_guard<decltype(inc)&&>{std::move(inc)}; // ... or even rvalue ref, which
                                                // with functions is treated
                                                // just like an lvalue...

  make_scope_guard(inc); // ... but the BEST is really to use the make function
}

////////////////////////////////////////////////////////////////////////////////
TEST_CASE("A plain-function-based scope_guard executes the function exactly "
          "once when leaving scope.")
{
  reset();

  {
    const auto guard = make_scope_guard(inc);
    REQUIRE_FALSE(count);
  }

  REQUIRE(count == 1u);
}

////////////////////////////////////////////////////////////////////////////////
TEST_CASE("An lvalue reference to a plain function can be used to create a "
          "scope_guard.")
{
  auto& inc_ref = inc;
  make_scope_guard(inc_ref);
}

////////////////////////////////////////////////////////////////////////////////
TEST_CASE("An lvalue-reference-to-plain-function-based scope_guard executes "
          "the function exactly once when leaving scope.")
{
  reset();

  {
    auto& inc_ref = inc;
    const auto guard = make_scope_guard(inc_ref);
    REQUIRE_FALSE(count);
  }

  REQUIRE(count == 1u);
}

////////////////////////////////////////////////////////////////////////////////
TEST_CASE("An rvalue reference to a plain function can be used to create a "
          "scope_guard.")
{
  make_scope_guard(std::move(inc)); // rvalue ref to function treated as lvalue
}

////////////////////////////////////////////////////////////////////////////////
TEST_CASE("An rvalue-reference-to-plain-function-based scope_guard executes "
          "the function exactly once when leaving scope.")
{
  reset();

  {
    const auto guard = make_scope_guard(std::move(inc));
    REQUIRE_FALSE(count);
  }

  REQUIRE(count == 1u);
}

#ifndef SG_REQUIRE_NOEXCEPT
////////////////////////////////////////////////////////////////////////////////
TEST_CASE("A reference wrapper to a plain function can be used to create a "
          "scope_guard.")
{
  make_scope_guard(std::ref(inc));
}

////////////////////////////////////////////////////////////////////////////////
TEST_CASE("A reference-wrapper-to-plain-function-based scope_guard executes "
          "the function exactly once when leaving scope.")
{
  reset();

  {
    const auto guard = make_scope_guard(std::ref(inc));
    REQUIRE_FALSE(count);
  }

  REQUIRE(count == 1u);
}

////////////////////////////////////////////////////////////////////////////////
TEST_CASE("A const reference wrapper to a plain function can be used to create "
          "a scope_guard.")
{
  make_scope_guard(std::cref(inc));
}

////////////////////////////////////////////////////////////////////////////////
TEST_CASE("A const-reference-wrapper-to-plain-function-based scope_guard "
          "executes the function exactly once when leaving scope.")
{
  reset();

  {
    const auto guard = make_scope_guard(std::cref(inc));
    REQUIRE_FALSE(count);
  }

  REQUIRE(count == 1u);
}
#endif

////////////////////////////////////////////////////////////////////////////////
TEST_CASE("An lvalue plain function pointer can be used to create a "
          "scope_guard.")
{
  const auto fp = &inc;
  make_scope_guard(fp);
}

////////////////////////////////////////////////////////////////////////////////
TEST_CASE("An lvalue-plain-function-pointer-based scope_guard executes the "
          "function exactly once when leaving scope.")
{
  reset();

  {
    const auto fp = &inc;
    const auto guard = make_scope_guard(fp);
    REQUIRE_FALSE(count);
  }

  REQUIRE(count == 1u);
}

////////////////////////////////////////////////////////////////////////////////
TEST_CASE("An rvalue plain function pointer can be used to create a "
          "scope_guard.")
{
  make_scope_guard(&inc);
}

////////////////////////////////////////////////////////////////////////////////
TEST_CASE("An rvalue-plain-function-pointer-based scope_guard executes the "
          "function exactly once when leaving scope.")
{
  reset();

  {
    const auto guard = make_scope_guard(&inc);
    REQUIRE_FALSE(count);
  }

  REQUIRE(count == 1u);
}

////////////////////////////////////////////////////////////////////////////////
TEST_CASE("An lvalue reference to a plain function pointer can be used to "
          "create a scope_guard.")
{
  const auto fp = &inc;
  const auto& fp_ref = fp;
  make_scope_guard(fp_ref);
}

////////////////////////////////////////////////////////////////////////////////
TEST_CASE("An plain-function-pointer-lvalue-reference-based scope_guard "
          "executes the function exactly once when leaving scope.")
{
  reset();

  {
    const auto fp = &inc;
    const auto& fp_ref = fp;
    const auto guard = make_scope_guard(fp_ref);
    REQUIRE_FALSE(count);
  }

  REQUIRE(count == 1u);
}

////////////////////////////////////////////////////////////////////////////////
TEST_CASE("An rvalue reference to a plain function pointer can be used to "
          "create a scope_guard.")
{
  const auto fp = &inc;
  make_scope_guard(std::move(fp));
}

////////////////////////////////////////////////////////////////////////////////
TEST_CASE("An plain-function-pointer-rvalue-reference-based scope_guard "
          "executes the function exactly once when leaving scope.")
{
  reset();

  {
    const auto fp = &inc;
    const auto guard = make_scope_guard(std::move(fp));
    REQUIRE_FALSE(count);
  }

  REQUIRE(count == 1u);
}

#ifndef SG_REQUIRE_NOEXCEPT
////////////////////////////////////////////////////////////////////////////////
TEST_CASE("An lvalue std::function that wraps a regular function can be used "
          "to create a scope_guard.")
{
  const auto stdf = make_std_function(inc);
  make_scope_guard(stdf);
}

////////////////////////////////////////////////////////////////////////////////
TEST_CASE("A scope_guard that is created with a "
          "regular-function-wrapping lvalue std::function executes that "
          "std::function exactly once when leaving scope.")
{
  reset();

  {
    REQUIRE_FALSE(count);
    const auto stdf = make_std_function(inc);
    const auto guard = make_scope_guard(stdf);
    REQUIRE_FALSE(count);
  }

  REQUIRE(count == 1u);
}

////////////////////////////////////////////////////////////////////////////////
TEST_CASE("An rvalue std::function that wraps a regular function can be used "
          "to create a scope_guard.")
{
  make_scope_guard(make_std_function(inc));
  make_scope_guard(std::function<void()>{inc});
}

////////////////////////////////////////////////////////////////////////////////
TEST_CASE("A scope_guard that is created with an "
          "regular-function-wrapping rvalue std::function executes that "
          "std::function exactly once when leaving scope.")
{
  reset();

  {
    REQUIRE_FALSE(count);
    const auto guard =
        make_scope_guard(make_std_function(inc));
    REQUIRE_FALSE(count);
  }

  REQUIRE(count == 1u);
}

////////////////////////////////////////////////////////////////////////////////
TEST_CASE("An lvalue reference to a std::function that wraps a regular "
          "function can be used to create a scope_guard.")
{
  const auto stdf = make_std_function(inc);
  const auto& stdf_ref = stdf;
  make_scope_guard(stdf_ref);
}

////////////////////////////////////////////////////////////////////////////////
TEST_CASE("A scope_guard that is created with an "
          "regular-function-wrapping std::function lvalue reference "
          "executes that std::function exactly once when leaving scope.")
{
  reset();

  {
    REQUIRE_FALSE(count);
    const auto stdf = make_std_function(inc);
    const auto& stdf_ref = stdf;
    const auto guard = make_scope_guard(stdf_ref);
    REQUIRE_FALSE(count);
  }

  REQUIRE(count == 1u);
}

////////////////////////////////////////////////////////////////////////////////
TEST_CASE("An rvalue reference to a std::function that wraps a regular "
          "function can be used to create a scope_guard.")
{
  const auto stdf = make_std_function(inc);
  make_scope_guard(std::move(stdf));
}

////////////////////////////////////////////////////////////////////////////////
TEST_CASE("A scope_guard that is created with an "
          "regular-function-wrapping std::function rvalue reference"
          "executes that std::function exactly once when leaving scope.")
{
  reset();

  {
    REQUIRE_FALSE(count);
    const auto stdf = make_std_function(inc);
    const auto guard = make_scope_guard(std::move(stdf));
    REQUIRE_FALSE(count);
  }

  REQUIRE(count == 1u);
}
#endif

////////////////////////////////////////////////////////////////////////////////
namespace
{
  auto lambda_no_capture_count = 0u;
}

////////////////////////////////////////////////////////////////////////////////
TEST_CASE("A lambda function with no capture can be used to create a "
          "scope_guard.")
{
  const auto guard = make_scope_guard([]()noexcept{});
}

////////////////////////////////////////////////////////////////////////////////
TEST_CASE("A no-capture-lambda-based scope_guard executes the lambda exactly "
          "once when leaving scope.")
{
  {
    REQUIRE_FALSE(lambda_no_capture_count);
    const auto guard = make_scope_guard([]() noexcept
                                        { incc(lambda_no_capture_count); });
    REQUIRE_FALSE(lambda_no_capture_count);
  }

  REQUIRE(lambda_no_capture_count == 1u);
}

////////////////////////////////////////////////////////////////////////////////
TEST_CASE("A lambda function with capture can be used to create a scope_guard.")
{
  make_scope_guard([]()noexcept{});
}

////////////////////////////////////////////////////////////////////////////////
TEST_CASE("A capturing-lambda-based scope_guard executes the lambda when "
          "leaving scope.")
{
  auto lambda_count = 0u;

  {
    const auto guard = make_scope_guard([&lambda_count]() noexcept
                                        { incc(lambda_count); });
    REQUIRE_FALSE(lambda_count);
  }

  REQUIRE(lambda_count == 1u);
}

////////////////////////////////////////////////////////////////////////////////
TEST_CASE("A scope_guard created with a regular-function-calling lambda, "
          "calls the lambda exactly once when leaving scope, which in turn "
          "calls the regular function.")
{
  reset();
  auto lambda_count = 0u;

  {
    const auto guard = make_scope_guard([&lambda_count]() noexcept
                                        { inc(); incc(lambda_count); });
    REQUIRE_FALSE(count);
    REQUIRE_FALSE(lambda_count);
  }

  REQUIRE(count == lambda_count);
  REQUIRE(count == 1u);
}

////////////////////////////////////////////////////////////////////////////////
TEST_CASE("A lambda function calling a std::function can be used to create a "
          "scope_guard.")
{
  make_scope_guard([]()noexcept{ make_std_function(inc)(); });
}

////////////////////////////////////////////////////////////////////////////////
TEST_CASE("A scope_guard created with a std::function-calling lambda calls "
          "the lambda exactly once when leaving scope, which in turn calls the "
          "std::function.")
{
  reset();
  auto lambda_count = 0u;

  {
    const auto guard = make_scope_guard(
      [&lambda_count]() noexcept
      {
        incc(lambda_count);
        make_std_function(inc)();
      });

    REQUIRE_FALSE(count);
    REQUIRE_FALSE(lambda_count);
  }

  REQUIRE(count == lambda_count);
  REQUIRE(count == 1u);
}

#ifndef SG_REQUIRE_NOEXCEPT
////////////////////////////////////////////////////////////////////////////////
TEST_CASE("A std::function wrapping a lambda function can be used to create a "
          "scope_guard.")
{
  make_scope_guard(std::function<void()>([](){}));
}

////////////////////////////////////////////////////////////////////////////////
TEST_CASE("A scope_guard created with a lambda-wrapping std::function calls "
          "the std::function exactly once when leaving scope.")
{
  reset();

  {
    const auto guard = make_scope_guard(std::function<void()>([](){ inc(); }));
    REQUIRE_FALSE(count);
  }

  REQUIRE(count == 1u);
}

////////////////////////////////////////////////////////////////////////////////
TEST_CASE("A bound function can be used to create a scope_guard.")
{
  auto boundf_count = 0u;
  make_scope_guard(std::bind(incc, std::ref(boundf_count)));
}

////////////////////////////////////////////////////////////////////////////////
TEST_CASE("A bound-function-based scope_guard calls the bound function exactly "
          "once when leaving scope.")
{
  auto boundf_count = 0u;

  {
    const auto guard = make_scope_guard(std::bind(incc,
                                                  std::ref(boundf_count)));
    REQUIRE_FALSE(boundf_count);
  }

  REQUIRE(boundf_count == 1u);
}

////////////////////////////////////////////////////////////////////////////////
TEST_CASE("A bound lambda can be used to create a scope_guard.")
{
  make_scope_guard(std::bind([](int /*unused*/){}, 42));
}

////////////////////////////////////////////////////////////////////////////////
TEST_CASE("A bound-lambda-based scope_guard calls the bound lambda exactly "
          "once when leaving scope.")
{
  auto boundl_count = 0u;

  {
    const auto incc_l = [](unsigned& c){ incc(c); };
    const auto guard = make_scope_guard(std::bind(incc_l,
                                                  std::ref(boundl_count)));
    REQUIRE_FALSE(boundl_count);
  }

  REQUIRE(boundl_count == 1u);
}
#endif

////////////////////////////////////////////////////////////////////////////////
TEST_CASE("several levels of indirection involving lambdas, binds, "
          "std::functions, custom functors, and regular functions.")
{
  // TODO
}

////////////////////////////////////////////////////////////////////////////////
TEST_CASE("Redundant scope_guards do not interfere with each other - their "
          "combined post-condition holds.")
{
  reset();
  auto lambda_count = 0u;

  {
    const auto g1 = make_scope_guard([&lambda_count]() noexcept
                                     { inc(); incc(lambda_count); });
    REQUIRE_FALSE(count);
    REQUIRE_FALSE(lambda_count);
    const auto g2 = make_scope_guard([&lambda_count]() noexcept
                                     { incc(lambda_count); inc(); });
    REQUIRE_FALSE(count);
    REQUIRE_FALSE(lambda_count);
    const auto g3 = make_scope_guard(inc);
    REQUIRE_FALSE(count);
  }

  REQUIRE(count == 3u);
  REQUIRE(lambda_count == 2u);

  const auto g4 = make_scope_guard([&lambda_count]() noexcept
                                   { incc(lambda_count); inc(); });
  REQUIRE(count == 3u);
  REQUIRE(lambda_count == 2u);
}

////////////////////////////////////////////////////////////////////////////////
TEST_CASE("Multiple independent scope_guards do not interfere with each "
          "other - each of their post-conditions hold.")
{
  auto a = 0u;
  auto b = 0u;
  auto c = 0u;

  {
    auto guard_a = make_scope_guard([&a]() noexcept { incc(a); });
    REQUIRE_FALSE(a);
    REQUIRE_FALSE(b);
    REQUIRE_FALSE(c);
  }
  REQUIRE(a == 1u);
  REQUIRE_FALSE(b);
  REQUIRE_FALSE(c);

  {
    auto guard_b = make_scope_guard([&b]() noexcept { incc(b); });
    auto guard_c = make_scope_guard([&c]() noexcept { incc(c); });
    REQUIRE(a == 1u);
    REQUIRE_FALSE(b);
    REQUIRE_FALSE(c);
  }
  REQUIRE(a == 1u);
  REQUIRE(b == 1u);
  REQUIRE(c == 1u);
}

////////////////////////////////////////////////////////////////////////////////
TEST_CASE("Test nested scopes")
{
  auto lvl0_count  = 0u;
  auto lvl1_count  = 0u;
  auto lvl2a_count = 0u;
  auto lvl2b_count = 0u;
  auto lvl3a_count = 0u;
  auto lvl3b_count = 0u;
  auto lvl3c_count = 0u;

  const auto lvl0_guard =
    make_scope_guard([&lvl0_count]() noexcept { incc(lvl0_count); });
  REQUIRE_FALSE(lvl0_count);

  {
    const auto lvl1_guard =
      make_scope_guard([&lvl1_count]() noexcept { incc(lvl1_count); });

    {
      const auto lvl2a_guard =
        make_scope_guard([&lvl2a_count]() noexcept { incc(lvl2a_count); });
      REQUIRE_FALSE(lvl2a_count);

      {
        const auto lvl3a_guard =
          make_scope_guard([&lvl3a_count]() noexcept { incc(lvl3a_count); });
        REQUIRE_FALSE(lvl3a_count);
      }

      REQUIRE(lvl3a_count == 1);
      REQUIRE_FALSE(lvl2a_count);
    }

    REQUIRE(lvl2a_count == 1);
    REQUIRE_FALSE(lvl1_count);
    REQUIRE_FALSE(lvl0_count);

    {
      const auto lvl2b_guard =
        make_scope_guard([&lvl2b_count]() noexcept { incc(lvl2b_count); });
      REQUIRE_FALSE(lvl2b_count);

      {
        const auto lvl3b_guard =
          make_scope_guard([&lvl3b_count]() noexcept { incc(lvl3b_count); });
        REQUIRE_FALSE(lvl3b_count);

        const auto lvl3c_guard =
          make_scope_guard([&lvl3c_count]() noexcept { incc(lvl3c_count); });
        REQUIRE_FALSE(lvl3c_count);
      }

      REQUIRE(lvl3b_count == 1);
      REQUIRE(lvl3c_count == 1);
      REQUIRE_FALSE(lvl2b_count);

    }

    REQUIRE(lvl2b_count == 1);
    REQUIRE_FALSE(lvl1_count);
    REQUIRE_FALSE(lvl0_count);

  }

  REQUIRE(lvl1_count  == 1);
  REQUIRE(lvl2a_count == 1);
  REQUIRE(lvl2b_count == 1);
  REQUIRE(lvl3a_count == 1);
  REQUIRE(lvl3b_count == 1);
  REQUIRE(lvl3c_count == 1);
  REQUIRE_FALSE(lvl0_count);

}

////////////////////////////////////////////////////////////////////////////////
TEST_CASE("scope_guards execute their callback exactly once when leaving "
          "scope due to an exception")
{
  reset();
  auto countl = 0u;

  try
  {
    const auto guard = make_scope_guard(inc);
    const auto guardl = make_scope_guard([&countl]() noexcept { ++countl; });
    throw "foo";
  }
  catch(...)
  {
    REQUIRE(count == 1u);
    REQUIRE(countl == 1u);
  }
}

////////////////////////////////////////////////////////////////////////////////
namespace
{
  unsigned returning(unsigned ret)
  {
    const auto guard = make_scope_guard(inc);
    return ret;
  }
}

////////////////////////////////////////////////////////////////////////////////
TEST_CASE("scope_guards execute their callback exactly once when leaving "
          "scope due to a return")
{
  reset();

  REQUIRE(123 == returning(123));
  REQUIRE(count == 1u);
}

////////////////////////////////////////////////////////////////////////////////
TEST_CASE("When a scope_guard is move-constructed, the moved guard does not "
          "execute its callback when moved, nor when leaving scope, but the "
          "thus-constructed guard still does execute its callback when leaving "
          "scope.")
{
  reset();

  {
    auto source = make_scope_guard(inc);
    {
      using CB = decltype(source)::callback_type;
      detail::scope_guard<CB> dest{std::move(source)};
      REQUIRE_FALSE(count); // inc not executed with source move
    }
    REQUIRE(count == 1u); // inc executed with destruction of dest
  }

  REQUIRE(count == 1u); // inc not executed with destruction of source
}
