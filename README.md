# scope_guard [under construction]
## TLDR
A general, easy to use, and exhaustively tested C++11/14/17 scope guard maker 
function. Example:

```c++
auto guard = make_scope_guard(my_callback);
```

<a href="https://github.com/ricab/scope_guard/blob/master/scope_guard.hpp">Single header</a> (remaining code is for tests)

## Table of contents

- [scope_guard [under construction]](#scope-guard--under-construction-)
  * [TLDR](#tldr)
  * [Table of contents](#table-of-contents)
  * [Features](#features)
    + [Main features](#main-features)
    + [Other characteristics](#other-characteristics)
  * [Usage](#usage)
    + [Preconditions](#preconditions)
      - [no arguments](#no-arguments)
      - [void return](#void-return)
      - [no throw](#no-throw)
    + [Option `SG_REQUIRE_NOEXCEPT_IN_CPP17`](#option--sg-require-noexcept-in-cpp17-)
      - [Implications](#implications)
  * [Running the tests](#running-the-tests)
    + [Instructions](#instructions)

<small><i><a href='http://ecotrust-canada.github.io/markdown-toc/'>Table of contents generated with markdown-toc</a></i></small>


## Features

### Main features
- [x] &ge; C++11
- [x] Single function interface (`make_scope_guard`)
- [x] Fast (no added runtime `std::function` penalties)
- [x] General: accepts any callable that respects the preconditions
[below](#preconditions)
- [x] no implicitly ignored return (see [below](#void-return))
- [x] Option to enforce `noexcept` in C++17
(see [below](#option-sg_require_noexcept_in_cpp17))

### Other characteristics
- [x] No dependencies to use (besides &ge;C++11 compiler and standard library)
- [x] No macros to make guard - just write explicit lambda or bind or what have
you
- [ ] [Exhaustively](catch_tests.cpp) [tested](compile_time_noexcept_tests.cpp)
- [x] Unlicense(d)
- [x] `snake_case` style

## Usage
To use,  simply clone this repository, copy the header file within, and include 
it - there are no dependencies (besides a &ge;C++11 compiler). Then do something
like:

```c++
auto guard1 = make_scope_guard(my_callback);
auto guard2 = make_scope_guard([]()noexcept{ /* do something */ });
...
```

See [tests](catch_tests.cpp) for more examples.

### Preconditions

The callback that is used to create a `scope_guard` must respect the following preconditions.

#### no arguments
The callback must be invocable with no arguments. Use bind or a lambda to pass
something that takes arguments in its original form. For example:

```c++
void my_resource_release(Resource& r) noexcept;
make_scope_guard(my_resource_release); // ERROR: which resource?
make_scope_guard([&some_resource]() noexcept { my_resource_release(some_resource); }); // OK
```

#### void return

The callback must return void. Returning anything else is intentionally
forbidden. This forces the client to confirm their intention, by explicitly
writing code to ignore a return, if that really is what they want. For example:

```c++
bool foo() noexcept;
make_scope_guard(foo); // ERROR: does not return void
make_scope_guard([]() noexcept {/*bool ignored =*/ foo();}); // OK
```

The idea is not only to catch unintentional cases but also to highlight
intentional ones for code readers.

#### no throw

The callback _is required_ not to throw. If you would like to use something that
might throw, you can wrap it in a `try-catch` block, explicitly choosing what
to do with any exceptions that arise. For example:

```c++
bool throwing() { throw std::runtime_error{"some error"}; }
make_scope_guard([]() noexcept 
{
  try { throwing(); } catch(...) { /* choosing to ignore */ }
});
```

Notice that, by default, callbacks are not verified not to throw. Throwing from
a `scope_guard` callback results in a call to std::terminate. This follows the
same approach as custom deleters in such standard library types as `unique_ptr`
and `shared_ptr` (see `[unique.ptr.single.ctor]` and
`[unique.ptr.single.dtor]` in the C++ standard.).

I acknowledge that the destructor for `scope_guard` could
be conditionally `noexcept` instead, but that is not advisable either and could
create a false sense of safety (better _fail-fast_-ish, I suppose).

Personally, I prefer to enforce that the callback be
`noexcept` at compile-time, but this not only restricts acceptable callback
types, to my knowledge it is not even possible until C++17. That is because the
exception specification is not part of the type system
[until then](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2015/p0012r1.html).

### Option `SG_REQUIRE_NOEXCEPT_IN_CPP17`

If &ge;C++17 is used, the preprocessor macro
`SG_REQUIRE_NOEXCEPT_IN_CPP17` can be defined to
make `scope_guard`'s constructor require a
[nothrow invocable](http://en.cppreference.com/w/cpp/types/is_invocable)
at compile-time, e.g.

```c++
#define SG_REQUIRE_NOEXCEPT_IN_CPP17 // only meaningful in >=C++17
#include "scope_guard.hpp"
make_scope_guard([](){}); // ERROR: need noexcept
make_scope_guard([]() noexcept {}); // OK
```

Notice however, that this restricts the types that `scope_guard` accepts
considerably. That is one of the reasons why it is disabled by default. The
other is to maintain the same behavior as in &lt;C++17.

This option has no effect unless &ge;C++17 is used.

#### Implications

Unfortunately, even in C++17 things are not ideal, and information on
exception specification is not propagated to types like `std::function` or
the result of `std::bind` and `std::ref`. For instance, the following code
does not compile in C++17:

```c++
void f() noexcept { }
auto stdf_noexc = std::function<void(&)()noexcept>{f}; // ERROR (at least in g++ and clang++)
auto stdf_declt = std::function<decltype(f)>{f};       // ERROR (at least in g++ and clang++)
auto stdf = std::function<void()>{f};                  // fine, but drops noexcept info
```

Since `SG_REQUIRE_NOEXCEPT_IN_CPP17` means rejecting anything that
is not known to possess an `operator() noexcept`, the additional
safety sacrifices generality. Of course the user can still use functions and
lambdas to wrap anything else, e.g.:

    make_scope_guard([&foo]()noexcept{std::bind(bar, foo)})

## Running the tests
There are a few dependencies to execute the tests:
- Cmake (at least version 3.8)
- C++17 capable compiler (c++1z is fine)
- Catch2
    
### Instructions
(For GNU/Linux, should be analogous in other systems.)

1. Install [cmake](https://cmake.org/) (&ge; v3.8)
2. Get and install [Catch2](https://github.com/catchorg/Catch2):
    ```
    $ git clone https://github.com/catchorg/Catch2 <catch_src_dir>
    $ mkdir <catch_bin_dir>
    $ cd <catch_bin_dir>
    $ cmake <catch_src_dir>
    $ make
    $ make install
    ```
3. Clone and build this repo:
    ```
    $ git clone https://github.com/ricab/scope_guard.git <guard_src_dir>
    $ mkdir <guard_bin_dir>
    $ cd <guard_bin_dir>
    $ cmake [options] <guard_src_dir>
    $ make
    $ make test
    ```

This will run catch and compile-time tests with different combinations of
SG_REQUIRE_NOEXCEPT_IN_CPP17 and C++ standard, depending on compiler
capabilities. If the compiler supports exception specifications as part of the
type system ([P0012R1](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2015/p0012r1.html)),
both C++11 and C++17 cases are tested (cases X, Y, W, and Z in the table below).
Otherwise, only C++11 is tested (cases X and Y below). Notice that noexcept is
only effectively required in case Z.

| standard/pp-define                                   | c++11 | c++17  |
| ---------------------------------------------------- |:-----:|:------:|
| **SG_REQUIRE_NOEXCEPT_IN_CPP17 undefined**           | X     |   W    |
| **SG_REQUIRE_NOEXCEPT_IN_CPP17 defined**             | Y     |  *Z*   |
 
Note: to obtain more output (e.g. because there was a failure), run
`VERBOSE=1 make test_verbose` instead, to get the command lines used in
compilation tests as well as the test output.




