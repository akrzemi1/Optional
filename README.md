Optional
========

A library for optional (nullable) objects for C++11. This is the reference implementation of proposal N793 (see http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2013/n3793.html). Optional is now accepted into Library Fundamentals Technical Specification (see http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2014/n3848.html). The interface is based on Fernando Cacciola's Boost.Optional library (see http://www.boost.org/doc/libs/1_52_0/libs/optional/doc/html/index.html)


Usage
-----

```cpp
optional<int> readInt(); // this function may return int or a not-an-int

if (optional<int> oi = readInt()) // did I get a real int
  cout << "my int is: " << *oi;   // use my int
else
  cout << "I have no int";
```

For more usage examples and the overview see http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2013/n3527.html


Supported compilers
-------------------

Clang 3.2, G++ 4.7.2, G++ 4.8.1


Differences from N3527
----------------------

 - The constructor taking `initializer_list` argument is not `constexpr`. This is because `initializer_list` operations are not `constexpr` in C++11.
 - In G++ 4.7.2 and 4.8.0 member function `value_or` does not have rvalue reference overload. These compilers do not support rvalue overloding on `*this`. 
