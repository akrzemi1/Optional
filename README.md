Optional
========

A library for optional (nullable) objects for C++11. This is the reference implementation of proposal N3527 (see see http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2013/n3527.html). Optional is now accepted into C++14 with modified wording as N3672. The interface is based on Fernando Cacciola's Boost.Optional library (http://www.boost.org/doc/libs/1_52_0/libs/optional/doc/html/index.html)


Supported compilers
-------------------

Clang 3.2, GCC 4.6+


Usage
-----

For usage examples and the overview see http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2013/n3527.html


Differences from N3527
----------------------

 - The constructor taking `initializer_list` argument is not `constexpr`. This is because `initializer_list` operations are not `constexpr` in C++11.
 - In GCC versions up to, but not including, 4.8.1, member function `value_or` does not have rvalue reference overload. This is because rvalue overloading on `*this` is not supported in these versions of GCC.
 - In GCC 4.6, `optional<T>` isn't a literal type even for trivially destructible T's. This is because of a GCC bug which causes a crash when compiling with the proposed implementation.
