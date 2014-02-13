// Copyright (C) 2011 - 2012 Andrzej Krzemienski.
//
// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

# include "optional.hpp"

struct Val
{
  Val(){}
  Val( Val const & ){}
  Val( Val && ) noexcept {}
  
  Val & operator=( Val const & ) = delete;
  Val & operator=( Val && ) noexcept = delete;
};

struct Safe
{
  Safe(){}
  Safe( Safe const & ){}
  Safe( Safe && ) noexcept {}
    
  Safe & operator=( Safe const & ){ return *this; }
  Safe & operator=( Safe && ) noexcept { return *this; }
};

struct Unsafe
{
  Unsafe(){}
  Unsafe( Unsafe const & ){}
  Unsafe( Unsafe && ){}
    
  Unsafe & operator=( Unsafe const & ){ return *this; }
  Unsafe & operator=( Unsafe && ) { return *this; }
};

struct VoidNothrowBoth
{
  VoidNothrowBoth(VoidNothrowBoth&&) noexcept(true) {};
  void operator=(VoidNothrowBoth&&) noexcept(true) {}; // note void return type
};


static_assert(std::is_nothrow_move_constructible<Safe>::value, "WTF!");
static_assert(!std::is_nothrow_move_constructible<Unsafe>::value, "WTF!");

static_assert(std::is_assignable<Safe&, Safe&&>::value, "WTF!");
static_assert(!std::is_assignable<Val&, Val&&>::value, "WTF!");

static_assert(std::is_nothrow_move_assignable<Safe>::value, "WTF!");
static_assert(!std::is_nothrow_move_assignable<Unsafe>::value, "WTF!");

static_assert(std::is_nothrow_move_constructible<VoidNothrowBoth>::value, "WTF!");
static_assert(std::is_nothrow_move_assignable<VoidNothrowBoth>::value, "WTF!");

int main() { }
