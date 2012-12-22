// Copyright (C) 2011 - 2012 Andrzej Krzemienski.
//
// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
// The idea and interface is based on Boost.Optional library
// authored by Fernando Luis Cacciola Carballal

# include "optional.hpp"
# include <vector>
# include <iostream>
# include <functional>
# include <complex>



struct caller {
    template <class T> caller(T fun) { fun(); };
};
# define CAT2(X, Y) X ## Y
# define CAT(X, Y) CAT2(X, Y)
# define TEST(NAME) caller CAT(__VAR, __LINE__) = []

enum  State 
{
    sDefaultConstructed,
    sValueCopyConstructed,
    sValueMoveConstructed,
    sCopyConstructed,
    sMoveConstructed,
    sMoveAssigned,
    sCopyAssigned,
    sValueCopyAssigned,
    sValueMoveAssigned,
    sMovedFrom,
    sValueConstructed
};

struct OracleVal
{
    State s;
    int i;
    OracleVal(int i = 0) : s(sValueConstructed), i(i) {}
};

struct Oracle
{
    State s;
    OracleVal val;

    Oracle() : s(sDefaultConstructed) {}
    Oracle(const OracleVal& v) : s(sValueCopyConstructed), val(v) {}
    Oracle(OracleVal&& v) : s(sValueMoveConstructed), val(std::move(v)) {v.s = sMovedFrom;}
    Oracle(const Oracle& o) : s(sCopyConstructed), val(o.val) {}
    Oracle(Oracle&& o) : s(sMoveConstructed), val(std::move(o.val)) {o.s = sMovedFrom;}

    Oracle& operator=(const OracleVal& v) { s = sValueCopyConstructed; val = v; return *this; }
    Oracle& operator=(OracleVal&& v) { s = sValueMoveConstructed; val = std::move(v); v.s = sMovedFrom; return *this; }
    Oracle& operator=(const Oracle& o) { s = sCopyConstructed; val = o.val; return *this; }
    Oracle& operator=(Oracle&& o) { s = sMoveConstructed; val = std::move(o.val); o.s = sMovedFrom; return *this; }
};

struct Guard
{
    std::string val;
    Guard() : val{} {}
    explicit Guard(std::string s, int = 0) : val(s) {}
    Guard(const Guard&) = delete;
    Guard(Guard&&) = delete;
    void operator=(const Guard&) = delete;
    void operator=(Guard&&) = delete;
};

struct ExplicitStr
{
    std::string s;
    explicit ExplicitStr(const char* chp) : s(chp) {};
};

struct Date
{
    int i;
    Date() = delete;
    Date(int i) : i{i} {};
    Date(Date&& d) : i(d.i) { d.i = 0; }
    Date(const Date&) = delete;
    Date& operator=(const Date&) = delete;
    Date& operator=(Date&& d) { i = d.i; d.i = 0; return *this;};
};

bool operator==( Oracle const& a, Oracle const& b ) { return a.val.i == b.val.i; }
bool operator!=( Oracle const& a, Oracle const& b ) { return a.val.i != b.val.i; }


namespace tr2 = std::experimental;


TEST(disengaged_ctor)
{
    tr2::optional<int> o1;
    assert (!o1);

    tr2::optional<int> o2 = tr2::nullopt;
    assert (!o2);

    tr2::optional<int> o3 = o2;
    assert (!o3);

    assert (o1 == tr2::nullopt);
    assert (o1 == tr2::optional<int>{});
    assert (!o1);
    assert (bool(o1) == false);

    assert (o2 == tr2::nullopt);
    assert (o2 == tr2::optional<int>{});
    assert (!o2);
    assert (bool(o2) == false);

    assert (o3 == tr2::nullopt);
    assert (o3 == tr2::optional<int>{});
    assert (!o3);
    assert (bool(o3) == false);

    assert (o1 == o2);
    assert (o2 == o1);
    assert (o1 == o3);
    assert (o3 == o1);
    assert (o2 == o3);
    assert (o3 == o2);
};


TEST(value_ctor)
{
  OracleVal v;
  tr2::optional<Oracle> oo1(v);
  assert (oo1 != tr2::nullopt);
  assert (oo1 != tr2::optional<Oracle>{});
  assert (oo1 == tr2::optional<Oracle>{v});
  assert (!!oo1);
  assert (bool(oo1));
  // NA: assert (oo1->s == sValueCopyConstructed);
  assert (oo1->s == sMoveConstructed);
  assert (v.s == sValueConstructed);
  
  tr2::optional<Oracle> oo2(std::move(v));
  assert (oo2 != tr2::nullopt);
  assert (oo2 != tr2::optional<Oracle>{});
  assert (oo2 == oo1);
  assert (!!oo2);
  assert (bool(oo2));
  // NA: assert (oo2->s == sValueMoveConstructed);
  assert (oo2->s == sMoveConstructed);
  assert (v.s == sMovedFrom);

  {
      OracleVal v;
      tr2::optional<Oracle> oo1{tr2::emplace, v};
      assert (oo1 != tr2::nullopt);
      assert (oo1 != tr2::optional<Oracle>{});
      assert (oo1 == tr2::optional<Oracle>{v});
      assert (!!oo1);
      assert (bool(oo1));
      assert (oo1->s == sValueCopyConstructed);
      assert (v.s == sValueConstructed);

      tr2::optional<Oracle> oo2{tr2::emplace, std::move(v)};
      assert (oo2 != tr2::nullopt);
      assert (oo2 != tr2::optional<Oracle>{});
      assert (oo2 == oo1);
      assert (!!oo2);
      assert (bool(oo2));
      assert (oo2->s == sValueMoveConstructed);
      assert (v.s == sMovedFrom);
  }
};


TEST(assignment)
{
    tr2::optional<int> oi;
    oi = tr2::optional<int>{1};
    assert (*oi == 1);

    oi = tr2::nullopt;
    assert (!oi);

    oi = 2;
    assert (*oi == 2);

    oi = {};
    assert (!oi);
};


template <class T>
struct MoveAware
{
  T val;
  bool moved;
  MoveAware(T val) : val(val), moved(false) {}
  MoveAware(MoveAware const&) = delete;
  MoveAware(MoveAware&& rhs) : val(rhs.val), moved(rhs.moved) {
    rhs.moved = true;
  }
  MoveAware& operator=(MoveAware const&) = delete;
  MoveAware& operator=(MoveAware&& rhs) {
    val = (rhs.val);
    moved = (rhs.moved);
    rhs.moved = true;
    return *this;
  }
};

TEST(moved_from_state)
{
  // first, test mock:
  MoveAware<int> i{1}, j{2};
  assert (i.val == 1);
  assert (!i.moved);
  assert (j.val == 2);
  assert (!j.moved);
  
  MoveAware<int> k = std::move(i);
  assert (k.val == 1);
  assert (!k.moved);
  assert (i.val == 1);
  assert (i.moved);
  
  k = std::move(j);
  assert (k.val == 2);
  assert (!k.moved);
  assert (j.val == 2);
  assert (j.moved);
  
  // now, test optional
  tr2::optional<MoveAware<int>> oi{1}, oj{2};
  assert (oi);
  assert (!oi->moved);
  assert (oj);
  assert (!oj->moved);
  
  tr2::optional<MoveAware<int>> ok = std::move(oi);
  assert (ok);
  assert (!ok->moved);
  assert (oi);
  assert (oi->moved);
  
  ok = std::move(oj);
  assert (ok);
  assert (!ok->moved);
  assert (oj);
  assert (oj->moved);
};


TEST(copy_move_ctor_optional_int)
{
  tr2::optional<int> oi;
  tr2::optional<int> oj = oi;
  
  assert (!oj);
  assert (oj == oi);
  assert (oj == tr2::nullopt);
  assert (!bool(oj));
  
  oi = 1;
  tr2::optional<int> ok = oi;
  assert (!!ok);
  assert (bool(ok));
  assert (ok == oi);
  assert (ok != oj);
  assert (*ok == 1);
  
  tr2::optional<int> ol = std::move(oi);
  assert (!!ol);
  assert (bool(ol));
  assert (ol == oi);
  assert (ol != oj);
  assert (*ol == 1);
};


TEST(optional_optional)
{
  tr2::optional<tr2::optional<int>> oi1 = tr2::nullopt;
  assert (oi1 == tr2::nullopt);
  assert (!oi1);
  
  {
  tr2::optional<tr2::optional<int>> oi2 {tr2::emplace};
  assert (oi2 != tr2::nullopt);
  assert (bool(oi2));
  assert (*oi2 == tr2::nullopt);
  //assert (!(*oi2));
  //std::cout << typeid(**oi2).name() << std::endl;
  }
  
  {
  tr2::optional<tr2::optional<int>> oi2 {tr2::emplace, tr2::nullopt};
  assert (oi2 != tr2::nullopt);
  assert (bool(oi2));
  assert (*oi2 == tr2::nullopt);
  assert (!*oi2);
  }
  
  {
  tr2::optional<tr2::optional<int>> oi2 {tr2::optional<int>{}};
  assert (oi2 != tr2::nullopt);
  assert (bool(oi2));
  assert (*oi2 == tr2::nullopt);
  assert (!*oi2);
  }
  
  tr2::optional<int> oi;
  auto ooi = tr2::make_optional(oi);
  static_assert( std::is_same<tr2::optional<tr2::optional<int>>, decltype(ooi)>::value, "");

};

TEST(example_guard)
{
  using namespace tr2;
  //FAILS: optional<Guard> ogx(Guard("res1")); 
  //FAILS: optional<Guard> ogx = "res1"; 
  //FAILS: optional<Guard> ogx("res1"); 
  optional<Guard> oga;                     // Guard is non-copyable (and non-moveable)     
  optional<Guard> ogb(emplace, "res1");   // initialzes the contained value with "res1"  
  assert (bool(ogb));
  assert (ogb->val == "res1");
            
  optional<Guard> ogc(emplace);           // default-constructs the contained value
  assert (bool(ogc));
  assert (ogc->val == "");

  oga.emplace("res1");                     // initialzes the contained value with "res1"  
  assert (bool(oga));
  assert (oga->val == "res1");
  
  oga.emplace();                           // destroys the contained value and 
                                           // default-constructs the new one
  assert (bool(oga));
  assert (oga->val == "");
  
  oga = nullopt;                        // OK: make disengaged the optional Guard
  assert (!(oga));
  //FAILS: ogb = {};                          // ERROR: Guard is not Moveable
};


void process(){}
void process(int ){}


TEST(example1)
{
  using namespace tr2;
  optional<int> oi;                 // create disengaged object
  optional<int> oj = nullopt;          // alternative syntax
  oi = oj;                          // assign disengaged object
  optional<int> ok = oj;            // ok is disengaged

  if (oi)  assert(false);           // 'if oi is engaged...'
  if (!oi) assert(true);            // 'if oi is disengaged...'

  if (oi != nullopt) assert(false);    // 'if oi is engaged...'
  if (oi == nullopt) assert(true);     // 'if oi is disengaged...'

  assert(oi == ok);                 // two disengaged optionals compare equal
  
  ///////////////////////////////////////////////////////////////////////////
  optional<int> ol{1};              // ol is engaged; its contained value is 1
  ok = 2;                           // ok becomes engaged; its contained value is 2
  oj = ol;                          // oj becomes engaged; its contained value is 1

  assert(oi != ol);                 // disengaged != engaged
  assert(ok != ol);                 // different contained values
  assert(oj == ol);                 // same contained value
  assert(oi < ol);                  // disengaged < engaged
  assert(ol < ok);                  // less by contained value
  
  /////////////////////////////////////////////////////////////////////////////
  int i = *ol;                      // i obtains the value contained in ol
  assert (i == 1);
  *ol = 9;                          // the object contained in ol becomes 9
  assert(*ol == 9);
  assert(ol == make_optional(9));  
  
  ////////////////////////////////
  if (ol)                      
    process(*ol);                   // use contained value if present
  else
    process();                      // proceed without contained value
  
  /////////////////////////////////////////
  process(get_value_or(ol, 0));     // use 0 if ol is disengaged
  
  ////////////////////////////////////////////
  ok = nullopt;                         // if ok was engaged calls T's dtor
  oj = {};                           // assigns a temporary disengaged optional
};


TEST(example_guard) 
{
  using std::experimental::optional;
  const optional<int> c = 4; 
  int i = *c;                        // i becomes 4
  assert (i == 4);
  // FAILS: *c = i;                            // ERROR: cannot assign to const int&
};


TEST(example_ref)
{
  using namespace std::experimental;
  int i = 1;
  int j = 2;
  optional<int&> ora;                 // disengaged optional reference to int
  optional<int&> orb = i;             // contained reference refers to object i

  *orb = 3;                          // i becomes 3
  // FAILS: ora = j;                           // ERROR: optional refs do not have assignment from T
  // FAILS: ora = {j};                         // ERROR: optional refs do not have copy/move assignment
  // FAILS: ora = orb;                         // ERROR: no copy/move assignment
  ora.emplace(j);                    // OK: contained reference refers to object j
  ora.emplace(i);                    // OK: contained reference now refers to object i

  ora = nullopt;                        // OK: ora becomes disengaged
};


template <typename T>
T getValue( tr2::optional<T> newVal = tr2::nullopt, tr2::optional<T&> storeHere = tr2::nullopt )
{
  T cached;
  
  if (newVal) {
    cached = *newVal;
    
    if (storeHere) {
      *storeHere = *newVal; // LEGAL: assigning T to T
    }      
  }
  return cached;      
}

TEST(example_optional_arg)
{
  int iii = 0;
  iii = getValue<int>(iii, iii);
  iii = getValue<int>(iii);
  iii = getValue<int>();
  
  {
    using namespace std::experimental;
    optional<Guard> grd1{emplace, "res1", 1};   // guard 1 initialized
    optional<Guard> grd2;

    grd2.emplace("res2", 2);                     // guard 2 initialized
    grd1 = nullopt;                                 // guard 1 released

  }                                              // guard 2 released (in dtor)
};


std::tuple<Date, Date, Date> getStartMidEnd() { return std::tuple<Date, Date, Date>{Date{1}, Date{2}, Date{3}}; }
void run(Date const&, Date const&, Date const&) {}

TEST(example_date)
{
  using namespace std::experimental;
  optional<Date> start, mid, end;           // Date doesn't have default ctor (no good default date)

  std::tie(start, mid, end) = getStartMidEnd();
  run(*start, *mid, *end); 
};


std::experimental::optional<char> readNextChar(){ return{}; }

void run(std::experimental::optional<std::string>) {}
void run(std::complex<double>) {}


template <class T>
void assign_norebind(tr2::optional<T&>& optref, T& obj)
{
  if (optref) *optref = obj;
  else        optref.emplace(obj);
}


TEST(example_rationale)
{
  using namespace std::experimental;
  if (optional<char> ch = readNextChar()) {
    // ...
  }
  
  //////////////////////////////////
  optional<int> opt1 = nullopt; 
  optional<int> opt2 = {}; 

  opt1 = nullopt;
  opt2 = {};

  if (opt1 == nullopt) {}
  if (!opt2) {}
  if (opt2 == optional<int>{}) {}
  
  
  
  ////////////////////////////////

  run(nullopt);            // pick the second overload
  // FAILS: run({});              // ambiguous

  if (opt1 == nullopt) {} // fine
  // FAILS: if (opt2 == {}) {}   // ilegal
  
  ////////////////////////////////
  assert (optional<unsigned>{}  < optional<unsigned>{0});
  assert (optional<unsigned>{0} < optional<unsigned>{1});
  assert (!(optional<unsigned>{}  < optional<unsigned>{}) );
  assert (!(optional<unsigned>{1} < optional<unsigned>{1}));

  assert (optional<unsigned>{}  != optional<unsigned>{0});
  assert (optional<unsigned>{0} != optional<unsigned>{1});
  assert (optional<unsigned>{}  == optional<unsigned>{} );
  assert (optional<unsigned>{0} == optional<unsigned>{0});
  
  /////////////////////////////////
  optional<int> o;
  o = make_optional(1);         // copy/move assignment
  o = 1;           // assignment from T
  o.emplace(1);    // emplacement 
  
  ////////////////////////////////////
  int isas = 0, i = 9;
  optional<int&> asas = i;
  assign_norebind(asas, isas);
  
  /////////////////////////////////////
  ////tr2::optional<std::vector<int>> ov2 = {2, 3};
  ////assert (bool(ov2));
  ////assert ((*ov2)[1] == 3);
  ////
  ////////////////////////////////
  ////std::vector<int> v = {1, 2, 4, 8};
  ////optional<std::vector<int>> ov = {1, 2, 4, 8};

  ////assert (v == *ov);
  ////
  ////ov = {1, 2, 4, 8};

  ////std::allocator<int> a;
  ////optional<std::vector<int>> ou { emplace, {1, 2, 4, 8}, a };

  ////assert (ou == ov);

  //////////////////////////////
  // inconvenient syntax:
  {
    
      tr2::optional<std::vector<int>> ov2{tr2::emplace, {2, 3}};
    
      assert (bool(ov2));
      assert ((*ov2)[1] == 3);
  
      ////////////////////////////

      std::vector<int> v = {1, 2, 4, 8};
      optional<std::vector<int>> ov{tr2::emplace, {1, 2, 4, 8}};

      assert (v == *ov);
  
      ov.emplace({1, 2, 4, 8});
/*
      std::allocator<int> a;
      optional<std::vector<int>> ou { emplace, {1, 2, 4, 8}, a };

      assert (ou == ov);
*/
  }

  /////////////////////////////////
  {
  typedef int T;
  optional<optional<T>> ot {emplace};
  optional<optional<T>> ou {emplace, nullopt};
  optional<optional<T>> ov {optional<T>{}};
  
  optional<int> oi;
  auto ooi = make_optional(oi);
  static_assert( std::is_same<optional<optional<int>>, decltype(ooi)>::value, "");
  }
};


TEST(bad_comparison)
{
  tr2::optional<int> oi, oj;
  int i;
  bool b = (oi == oj);
  // FAILS: b = (oi >= i);
  // FAILS: b = (oi == i);
};


//// NOT APPLICABLE ANYMORE
////TEST(perfect_ctor)
////{
////  //tr2::optional<std::string> ois = "OS";
////  assert (*ois == "OS");
////  
////  // FAILS: tr2::optional<ExplicitStr> oes = "OS"; 
////  tr2::optional<ExplicitStr> oes{"OS"};
////  assert (oes->s == "OS");
////};

TEST(get_value_or)
{
  tr2::optional<int> oi = 1;
  int i = get_value_or(oi, 0);
  assert (i == 1);
  
  oi = tr2::nullopt;
  assert (get_value_or(oi, 3) == 3);
  
  tr2::optional<std::string> os{"AAA"};
  assert (get_value_or(os, "BBB") == "AAA");
  os = {};
  assert (get_value_or(os, "BBB") == "BBB");
};

TEST(optional_ref)
{
  using namespace tr2;
  // FAILS: optional<int&&> orr;
  // FAILS: optional<nullopt_t&> on;
  int i = 8;
  optional<int&> ori;
  assert (!ori);
  ori.emplace(i);
  assert (bool(ori));
  assert (*ori == 8);
  *ori = 9;
  assert (i == 9);
  
  // FAILS: int& ir = get_value_or(ori, i);
  int ii = get_value_or(ori, i);
  assert (ii == 9);
  ii = 7;
  assert (*ori == 9);
  
  int j = 22;
  auto&& oj = make_optional(std::ref(j));
  *oj = 23;
  assert (j == 23);
};


TEST(optional_initialization)
{
    using namespace tr2;
    using std::string;
    string s = "STR";

    optional<string> os{s};
    optional<string> ot = s;
    optional<string> ou{"STR"};
    optional<string> ov = string{"STR"};
    
};

#include <unordered_set>

TEST(optional_hashing)
{
    using namespace tr2;
    using std::string;
    
    std::hash<int> hi;
    std::hash<optional<int>> hoi;
    std::hash<string> hs;
    std::hash<optional<string>> hos;
    
    assert (hi(0) == hoi(optional<int>{0}));
    assert (hi(1) == hoi(optional<int>{1}));
    assert (hi(3198) == hoi(optional<int>{3198}));
    
    assert (hs("") == hos(optional<string>{""}));
    assert (hs("0") == hos(optional<string>{"0"}));
    assert (hs("Qa1#") == hos(optional<string>{"Qa1#"}));
    
    std::unordered_set<optional<string>> set;
    assert(set.find({"Qa1#"}) == set.end());
    
    set.insert({"0"});
    assert(set.find({"Qa1#"}) == set.end());
    
    set.insert({"Qa1#"});
    assert(set.find({"Qa1#"}) != set.end());
};


//// constexpr tests

// these 4 classes have different noexcept signatures in move operations
struct NothrowBoth {
  NothrowBoth(NothrowBoth&&) noexcept(true) {};
  void operator=(NothrowBoth&&) noexcept(true) {};
};
struct NothrowCtor {
  NothrowCtor(NothrowCtor&&) noexcept(true) {};
  void operator=(NothrowCtor&&) noexcept(false) {};
};
struct NothrowAssign {
  NothrowAssign(NothrowAssign&&) noexcept(false) {};
  void operator=(NothrowAssign&&) noexcept(true) {};
};
struct NothrowNone {
  NothrowNone(NothrowNone&&) noexcept(false) {};
  void operator=(NothrowNone&&) noexcept(false) {};
};

void test_noexcept()
{
  {
    tr2::optional<NothrowBoth> b1, b2;
    static_assert(noexcept(tr2::optional<NothrowBoth>{std::move(b1)}), "bad noexcept!");
    static_assert(noexcept(b1 = std::move(b2)), "bad noexcept!");
  }
  {
    tr2::optional<NothrowCtor> c1, c2;
    static_assert(noexcept(tr2::optional<NothrowCtor>{std::move(c1)}), "bad noexcept!");
    static_assert(!noexcept(c1 = std::move(c2)), "bad noexcept!");
  }
  {
    tr2::optional<NothrowAssign> a1, a2;
    static_assert(!noexcept(tr2::optional<NothrowAssign>{std::move(a1)}), "bad noexcept!");
    static_assert(!noexcept(a1 = std::move(a2)), "bad noexcept!");
  }
  {
    tr2::optional<NothrowNone> n1, n2;
    static_assert(!noexcept(tr2::optional<NothrowNone>{std::move(n1)}), "bad noexcept!");
    static_assert(!noexcept(n1 = std::move(n2)), "bad noexcept!");
  }
}


void constexpr_test_disengaged()
{
  constexpr tr2::optional<int> g0{};
  constexpr tr2::optional<int> g1{tr2::nullopt};
  static_assert( !g0, "initialized!" );
  static_assert( !g1, "initialized!" );
  
  static_assert( bool(g1) == bool(g0), "ne!" );
  
  static_assert( g1 == g0, "ne!" );
  static_assert( !(g1 != g0), "ne!" );
  static_assert( g1 >= g0, "ne!" );
  static_assert( !(g1 > g0), "ne!" );
  static_assert( g1 <= g0, "ne!" );
  static_assert( !(g1 <g0), "ne!" );
  
  static_assert( g1 == tr2::nullopt, "!" );
  static_assert( !(g1 != tr2::nullopt), "!" );
  static_assert( g1 <= tr2::nullopt, "!" );
  static_assert( !(g1 < tr2::nullopt), "!" );
  static_assert( g1 >= tr2::nullopt, "!" );
  static_assert( !(g1 > tr2::nullopt), "!" );
  
  static_assert(  (tr2::nullopt == g0), "!" );
  static_assert( !(tr2::nullopt != g0), "!" );
  static_assert(  (tr2::nullopt >= g0), "!" );
  static_assert( !(tr2::nullopt >  g0), "!" );
  static_assert(  (tr2::nullopt <= g0), "!" );
  static_assert( !(tr2::nullopt <  g0), "!" );
  
  static_assert(  (g1 != tr2::make_optional(1)), "!" );
  static_assert( !(g1 == tr2::make_optional(1)), "!" );
  static_assert(  (g1 <  tr2::make_optional(1)), "!" );
  static_assert(  (g1 <= tr2::make_optional(1)), "!" );
  static_assert( !(g1 >  tr2::make_optional(1)), "!" );
  static_assert( !(g1 >  tr2::make_optional(1)), "!" );
}


constexpr tr2::optional<int> g0{};
constexpr tr2::optional<int> g2{2};
static_assert( g2, "not initialized!" );
static_assert( *g2 == 2, "not 2!" );
static_assert( g2 == tr2::make_optional(2), "not 2!" );
static_assert( g2 != g0, "eq!" );


// end constexpr tests


#include <string>


struct VEC
{
    std::vector<int> v;
    template <typename... X>
    VEC( X&&...x) : v(std::forward<X>(x)...) {};

    template <typename U, typename... X>
    VEC(std::initializer_list<U> il, X&&...x) : v(il, std::forward<X>(x)...) {};
};


int main() {
    tr2::optional<int> oi = 1;
    assert (bool(oi));
    oi.operator=({});
    assert (!oi);
    std::cout << "OK" << std::endl;

    VEC v = {5, 6};

    std::cout << 4 << std::endl;
    
  // type optional<string> is deduced
  //tr2::optional<std::vector<int>> ov2 = {2, 3};

	//std::allocator<int> a;
  //tr2::optional<std::vector<int>> ov{ tr2::emplace, {1, 3, 5}, a };

//ov = {7,8,9,0}	;
//  std::cout << bool(ov) << " " << ov->size() << " " << (*ov)[2] << std::endl;

}

