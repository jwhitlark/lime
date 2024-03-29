// C headers
#include <cstdlib>
#include <ctime>

// STL headers
#include <iostream>
#include <sstream>

// lime headers
#include <builtins.hpp>
#include <eval.hpp>
#include <interpreter.hpp>
#include <parse.hpp>

namespace lime {
  // STL
  using std::cin;
  using std::cout;
  using std::getline;
  using std::stringstream;
  using std::make_shared;

  // Boost
  using boost::apply_visitor;
  using boost::static_visitor;

  // lime
  using lime::check;
  using lime::escape;
  using lime::eval;
  using lime::nil;
  using lime::output;
  using lime::parse;
  using lime::reference_visitor;
  using lime::unescape;

  value quote::call(vector< value > args, shared_ptr< environment > caller_env_p)
  {
    check(args.size() == 1, "wrong number of arguments to 'quote' (must be 1).");
    return args.front();
  }

  value evaluate::call(vector< value > args, shared_ptr< environment > caller_env_p)
  {
    check(args.size() == 1, "wrong number of arguments to 'eval' (must be 1).");
    return eval(eval(args.front(), caller_env_p), caller_env_p);
  }

  value make_list::call(vector< value > args, shared_ptr< environment > caller_env_p)
  {
    list lst;
    for (value arg: args)
      lst.push_back(eval(arg, caller_env_p));
    return lst;
  }  

  class load_visitor : public static_visitor<> {
  public:
    load_visitor(shared_ptr< environment > ep) : env_p(ep) {}
    void operator()(const string& path) const
    {
      load_file(path, env_p);
    }
    template< typename T >
    void operator()(const T& t) const
    {
      check(false, "argument to 'load' must be a string.");
    }
  private:
    shared_ptr< environment > env_p;
  };

  value load::call(vector< value > args, shared_ptr< environment > caller_env_p)
  {
    check(args.size() == 1, "wrong number of arguments to 'load' (must be 1).");
    apply_visitor(load_visitor(caller_env_p), args.front());
    return nil();
  }

  class equals_visitor : public static_visitor< bool > {
  public:
    bool operator()(int a, int b) const
    {
      return a == b;
    }
    bool operator()(const string& a, const string& b) const
    {
      return a == b;
    }
    bool operator()(bool a, bool b) const
    {
      return a == b;
    }
    bool operator()(const list& a, const list& b) const
    {
      if (a.empty())
        return b.empty();
      else if (b.empty())
        return false;
      else {
        equals_visitor eq;
        value ah(a.head()), bh(b.head());
        value at(a.tail()), bt(b.tail());
        return (apply_visitor(eq, ah, bh) && apply_visitor(eq, at, bt));
      }
    }
    template< typename T, typename U >
    bool operator()(const T& a, const U& b) const
    {
      return false;
    }
  };

  class equals_partial : public lambda {
  public:
    equals_partial(value a1) : arg1(a1) {}
    value call(vector< value > args, shared_ptr< environment > caller_env_p)
    {
      check(args.size() == 1, "wrong number of arguments to '= <expr>' (must be 1).");
      value arg2 = eval(args.front(), caller_env_p);
      return apply_visitor(equals_visitor(), arg1, arg2);
    }
  private:
    value arg1;
  };

  value equals::call(vector< value > args, shared_ptr< environment > caller_env_p)
  {
    check(args.size() == 1 || args.size() == 2, 
          "wrong number of arguments to '=' (must be 1 or 2).");
    value arg1 = eval(args[0], caller_env_p);
    if (args.size() == 1)
      return make_shared< equals_partial >(arg1);
    value arg2 = eval(args[1], caller_env_p);
    return apply_visitor(equals_visitor(), arg1, arg2);
  }

  class less_than_visitor : public static_visitor< bool > {
  public:
    bool operator()(int a, int b) const
    {
      return a < b;
    }
    template< typename T, typename U >
    bool operator()(const T& a, const U& b) const
    {
      check(false, "arguments to '<' must be integer.");
    }
  };

  class less_than_partial : public lambda {
  public:
    less_than_partial(value a1) : arg1(a1) {}
    value call(vector< value > args, shared_ptr< environment > caller_env_p)
    {
      check(args.size() == 1, "wrong number of arguments to '< <expr>' (must be 1).");
      value arg2 = eval(args.front(), caller_env_p);
      return apply_visitor(less_than_visitor(), arg1, arg2);
    }
  private:
    value arg1;
  };

  value less_than::call(vector< value > args, shared_ptr< environment > caller_env_p)
  {
    check(args.size() == 1 || args.size() == 2, 
          "wrong number of arguments to '<' (must be 1 or 2).");
    value arg1 = eval(args[0], caller_env_p);
    if (args.size() == 1)
      return make_shared< less_than_partial >(arg1);
    value arg2 = eval(args[1], caller_env_p);
    return apply_visitor(less_than_visitor(), arg1, arg2);
  }

  class plus_visitor : public static_visitor< int > {
  public:
    int operator()(int a, int b) const
    {
      return a + b;
    }
    template< typename T, typename U >
    int operator()(const T& a, const U& b) const
    {
      check(false, "arguments to '+' must be integer.");
    }
  };

  class plus_partial : public lambda {
  public:
    plus_partial(value a1) : arg1(a1) {}
    value call(vector< value > args, shared_ptr< environment > caller_env_p)
    {
      check(args.size() == 1, "wrong number of arguments to '+ <expr>' (must be 1).");
      value arg2 = eval(args.front(), caller_env_p);
      return apply_visitor(plus_visitor(), arg1, arg2);
    }
  private:
    value arg1;
  };

  value plus::call(vector< value > args, shared_ptr< environment > caller_env_p)
  {
    check(args.size() == 1 || args.size() == 2, 
          "wrong number of arguments to '+' (must be 1 or 2).");
    value arg1 = eval(args[0], caller_env_p);
    if (args.size() == 1)
      return make_shared< plus_partial >(arg1);
    value arg2 = eval(args[1], caller_env_p);
    return apply_visitor(plus_visitor(), arg1, arg2);   
  }

  class minus_visitor : public static_visitor< int > {
  public:
    int operator()(int a, int b) const
    {
      return a - b;
    }
    template< typename T, typename U >
    int operator()(const T& a, const U& b) const
    {
      check(false, "arguments to '-' must be integer.");
    }
  };

  class minus_partial : public lambda {
  public:
    minus_partial(value a1) : arg1(a1) {}
    value call(vector< value > args, shared_ptr< environment > caller_env_p)
    {
      check(args.size() == 1, "wrong number of arguments to '- <expr>' (must be 1).");
      value arg2 = eval(args.front(), caller_env_p);
      return apply_visitor(minus_visitor(), arg1, arg2);
    }
  private:
    value arg1;
  };

  value minus::call(vector< value > args, shared_ptr< environment > caller_env_p)
  {
    check(args.size() == 1 || args.size() == 2, 
          "wrong number of arguments to '-' (must be 1 or 2).");
    value arg1 = eval(args[0], caller_env_p);
    if (args.size() == 1)
      return make_shared< minus_partial >(arg1);
    value arg2 = eval(args[1], caller_env_p);
    return apply_visitor(minus_visitor(), arg1, arg2);   
  }

  class times_visitor : public static_visitor< int > {
  public:
    int operator()(int a, int b) const
    {
      return a * b;
    }
    template< typename T, typename U >
    int operator()(const T& a, const U& b) const
    {
      check(false, "arguments to '*' must be integer.");
    }
  };

  class times_partial : public lambda {
  public:
    times_partial(value a1) : arg1(a1) {}
    value call(vector< value > args, shared_ptr< environment > caller_env_p)
    {
      check(args.size() == 1, "wrong number of arguments to '* <expr>' (must be 1).");
      value arg2 = eval(args.front(), caller_env_p);
      return apply_visitor(times_visitor(), arg1, arg2);
    }
  private:
    value arg1;
  };

  value times::call(vector< value > args, shared_ptr< environment > caller_env_p)
  {
    check(args.size() == 1 || args.size() == 2, 
          "wrong number of arguments to '*' (must be 1 or 2).");
    value arg1 = eval(args[0], caller_env_p);
    if (args.size() == 1)
      return make_shared< times_partial >(arg1);
    value arg2 = eval(args[1], caller_env_p);
    return apply_visitor(times_visitor(), arg1, arg2);   
  }

  class divide_visitor : public static_visitor< int > {
  public:
    int operator()(int a, int b) const
    {
      check(b != 0, "second argument to '/' must be non-zero.");
      return a / b;
    }
    template< typename T, typename U >
    int operator()(const T& a, const U& b) const
    {
      check(false, "arguments to '/' must be integer.");
    }
  };

  class divide_partial : public lambda {
  public:
    divide_partial(value a1) : arg1(a1) {}
    value call(vector< value > args, shared_ptr< environment > caller_env_p)
    {
      check(args.size() == 1, "wrong number of arguments to '/ <expr>' (must be 1).");
      value arg2 = eval(args.front(), caller_env_p);
      return apply_visitor(divide_visitor(), arg1, arg2);
    }
  private:
    value arg1;
  };

  value divide::call(vector< value > args, shared_ptr< environment > caller_env_p)
  {
    check(args.size() == 1 || args.size() == 2, 
          "wrong number of arguments to '/' (must be 1 or 2).");
    value arg1 = eval(args[0], caller_env_p);
    if (args.size() == 1)
      return make_shared< divide_partial >(arg1);
    value arg2 = eval(args[1], caller_env_p);
    return apply_visitor(divide_visitor(), arg1, arg2);   
  }

  class modulo_visitor : public static_visitor< int > {
  public:
    int operator()(int a, int b) const
    {
      check(b != 0, "second argument to '%' must be non-zero.");
      return a % b;
    }
    template< typename T, typename U >
    int operator()(const T& a, const U& b) const
    {
      check(false, "arguments to '%' must be integer.");
    }
  };

  class modulo_partial : public lambda {
  public:
    modulo_partial(value a1) : arg1(a1) {}
    value call(vector< value > args, shared_ptr< environment > caller_env_p)
    {
      check(args.size() == 1, "wrong number of arguments to '% <expr>' (must be 1).");
      value arg2 = eval(args.front(), caller_env_p);
      return apply_visitor(modulo_visitor(), arg1, arg2);
    }
  private:
    value arg1;
  };

  value modulo::call(vector< value > args, shared_ptr< environment > caller_env_p)
  {
    check(args.size() == 1 || args.size() == 2, 
          "wrong number of arguments to '%' (must be 1 or 2).");
    value arg1 = eval(args[0], caller_env_p);
    if (args.size() == 1)
      return make_shared< modulo_partial >(arg1);
    value arg2 = eval(args[1], caller_env_p);
    return apply_visitor(modulo_visitor(), arg1, arg2);   
  }

  value random_int::call(vector< value > args, shared_ptr< environment > caller_env_p)  
  {
    return rand();
  }

  class bool_visitor : public static_visitor< bool > {
  public:
    bool operator()(bool a) const
    {
      return a;
    }
    template< typename T >
    bool operator()(const T& a) const
    {
      check(false, "boolean value expected.");
    }
  };

  class is_atom_visitor : public static_visitor< bool > {
  public:
    bool operator()(const list& lst) const
    {
      return false;
    }
    template< typename T >
    bool operator()(const T& t) const
    {
      return true;
    }
  };

  value is_atom::call(vector< value > args, shared_ptr< environment > caller_env_p)
  {
    check(args.size() == 1, "wrong number of arguments to 'atom?' (must be 1).");
    value arg = eval(args.front(), caller_env_p);
    return apply_visitor(is_atom_visitor(), arg);
  }

  class len_visitor : public static_visitor< int > {
  public:
    int operator()(const list& lst) const
    {
      return lst.size();
    }
    template< typename T >
    int operator()(const T& t) const
    {
      check(false, "argument to 'len' must be a list.");
    }
  };

  value len::call(vector< value > args, shared_ptr< environment > caller_env_p)
  {
    check(args.size() == 1, "wrong number of arguments to 'len' (must be 1).");
    value arg = eval(args.front(), caller_env_p);
    return apply_visitor(len_visitor(), arg);
  }

  class cons_visitor : public static_visitor< list > {
  public:
    template< typename T >
    list operator()(const T& h, const list& t) const
    {
      return list(h, t);
    }
    template< typename T, typename U >
    list operator()(const T& t, const U& u) const
    {
      check(false, "arguments to 'cons' must be a value and a list.");
    }
  };

  class cons_partial : public lambda {
  public:
    cons_partial(value a1) : arg1(a1) {}
    value call(vector< value > args, shared_ptr< environment > caller_env_p)
    {
      check(args.size() == 1, "wrong number of arguments to 'cons <expr>' (must be 1).");
      value arg2 = eval(args.front(), caller_env_p);
      return apply_visitor(cons_visitor(), arg1, arg2);
    }
  private:
    value arg1;
  };

  value cons::call(vector< value > args, shared_ptr< environment > caller_env_p)
  {
    check(args.size() == 1 || args.size() == 2, 
          "wrong number of arguments to 'cons' (must be 1 or 2).");
    value arg1 = eval(args[0], caller_env_p);
    if (args.size() == 1)
      return make_shared< cons_partial >(arg1);
    value arg2 = eval(args[1], caller_env_p);
    return apply_visitor(cons_visitor(), arg1, arg2);   
  }

  class head_visitor : public static_visitor< value > {
  public:
    value operator()(const list& lst) const
    {
      check(!lst.empty(), "argument to 'head' must be a non-empty list.");
      return lst.head();
    }
    template< typename T >
    value operator()(const T& t) const
    {
      check(false, "argument to 'head' must be a non-empty list.");
    }
  };

  value head::call(vector< value > args, shared_ptr< environment > caller_env_p)
  {
    check(args.size() == 1, "wrong number of arguments to 'head' (must be 1).");
    value arg = eval(args.front(), caller_env_p);
    return apply_visitor(head_visitor(), arg);
  }

  class tail_visitor : public static_visitor< list > {
  public:
    list operator()(const list& lst) const
    {
      check(!lst.empty(), "argument to 'tail' must be a non-empty list.");
      return lst.tail();
    }
    template< typename T >
    list operator()(const T& t) const
    {
      check(false, "argument to 'tail' must be a non-empty list.");
    }
  };
  
  value tail::call(vector< value > args, shared_ptr< environment > caller_env_p)
  {
    check(args.size() == 1, "wrong number of arguments to 'tail' (must be 1).");
    value arg = eval(args.front(), caller_env_p);
    return apply_visitor(tail_visitor(), arg);
  }

  class elem_visitor : public static_visitor< value > {
  public:
    value operator()(int i, const list& lst) const
    {
      check(i >= 1 && i <= lst.size(), "list index out of range.");
      return lst[i - 1];
    }
    value operator()(const int i, const symbol& sym) const
    {
      check(false, "SYMBOL!");
    }
    value operator()(const int i, const shared_ptr<reference>& sym) const
    {
      check(false, "REF!");
    }
    template< typename T, typename U >
    value operator()(const T& t, const U& u) const
    {
      check(false, "arguments to 'elem' must be an integer index and a non-empty list.");
    }
  };

  class elem_partial : public lambda {
  public:
    elem_partial(value a1) : arg1(a1) {}
    value call(vector< value > args, shared_ptr< environment > caller_env_p)
    {
      check(args.size() == 1, "wrong number of arguments to 'elem <expr>' (must be 1).");
      value arg2 = eval(args.front(), caller_env_p);
      return apply_visitor(elem_visitor(), arg1, arg2);
    }
  private:
    value arg1;
  };

  value elem::call(vector< value > args, shared_ptr< environment > caller_env_p)
  {
    check(args.size() == 1 || args.size() == 2, 
          "wrong number of arguments to 'elem' (must be 1 or 2).");
    value arg1 = eval(args[0], caller_env_p);
    if (args.size() == 1)
      return make_shared< elem_partial >(arg1);
    value arg2 = eval(args[1], caller_env_p);
    return apply_visitor(elem_visitor(), arg1, arg2);   
  }

  class native_ref_visitor : public static_visitor< value& > {
  public:
    native_ref_visitor(shared_ptr< environment > ep) : env_p(ep) {}
    value& operator()(const symbol& sym) const
    {
      check(env_p->find(sym), "symbol '" + sym + "' not found.");
      return env_p->get_ref(sym);
    }
    template< typename T>
    value& operator()(const T& t) const
    {
      check(false, "attempting to get reference to non-symbol.");
    }
  private:
    shared_ptr< environment > env_p;
  };

  class elem_ref_visitor : public static_visitor< value& > {
  public:
    value& operator()(list& lst, int i) const
    {
      check(i >= 1 && i <= lst.size(), "list index out of range.");
      return lst[i - 1];
    }
    value& operator()(shared_ptr< reference >& lst_ref, int i) const
    {
      value& lst = lst_ref->get_native_ref();
      value i_v(i);
      return apply_visitor(elem_ref_visitor(), lst, i_v);
    }
    template< typename T, typename U >
    value& operator()(T& t, U u) const
    {
      check(false, "arguments to 'set-elem!' must be a reference to a non-empty list, "
            "an integer index, and a value.");
    }
  };

  class set_elem_partial2 : public lambda {
  public:
    set_elem_partial2(value a1, value a2, shared_ptr< environment > ep) 
      : arg1(a1), arg2(a2), env_p(ep) {}
    value call(vector< value > args, shared_ptr< environment > caller_env_p)
    {
      check(args.size() == 1, 
            "wrong number of arguments to 'set-elem! <expr> <expr>' (must be 1).");
      value& list_ref = apply_visitor(native_ref_visitor(env_p), arg1);
      value arg3 = eval(args.front(), caller_env_p);
      value& ref = apply_visitor(elem_ref_visitor(), list_ref, arg2);
      ref = arg3;
      return nil();
    }
  private:
    value arg1, arg2;
    shared_ptr< environment > env_p;
  };

  class set_elem_partial : public lambda {
  public:
    set_elem_partial(value a1, shared_ptr< environment > ep) : arg1(a1), env_p(ep) {}
    value call(vector< value > args, shared_ptr< environment > caller_env_p)
    {
      check(args.size() == 1 || args.size() == 2, 
            "wrong number of arguments to 'set-elem! <expr>' (must be 1 or 2).");
      value arg2 = eval(args.front(), caller_env_p);
      if (args.size() == 1)
        return make_shared< set_elem_partial2 >(arg1, arg2, env_p);
      value& list_ref = apply_visitor(native_ref_visitor(env_p), arg1);
      value arg3 = eval(args.back(), caller_env_p);
      value& ref = apply_visitor(elem_ref_visitor(), list_ref, arg2);
      ref = arg3;
      return nil();
    }
  private:
    value arg1;
    shared_ptr< environment > env_p;
  };

  value set_elem::call(vector< value > args, shared_ptr< environment > caller_env_p)
  {
    check(args.size() >= 1 && args.size() <= 3, 
          "wrong number of arguments to 'set-elem!' (must be 1, 2 or 3).");
    value arg1 = args[0];
    if (args.size() == 1)
      return make_shared< set_elem_partial >(arg1, caller_env_p);
    value arg2 = eval(args[1], caller_env_p);
    if (args.size() == 2)
      return make_shared< set_elem_partial2 >(arg1, arg2, caller_env_p);
    value& list_ref = apply_visitor(native_ref_visitor(caller_env_p), arg1);
    value arg3 = eval(args[2], caller_env_p);
    value& ref = apply_visitor(elem_ref_visitor(), list_ref, arg2);
    ref = arg3;
    return nil();
  }

  class push_front_visitor : public static_visitor<> {
  public:
    template< typename T>
    void operator()(list& lst, const T& val) const
    {
      lst.push_front(val);
    }
    template< typename T>
    void operator()(shared_ptr< reference >& lst_ref, const T& val) const
    {
      value& lst = lst_ref->get_native_ref();
      value v(val);
      apply_visitor(push_front_visitor(), lst, v);
    }
    template< typename T, typename U>
    void operator()(T& t, const U& u) const
    {
      check(false, 
            "arguments to 'push-front!' must be a reference to a list and a value.");
    }
  };

  class push_front_partial : public lambda {
  public:
    push_front_partial(value a1, shared_ptr< environment > ep) : arg1(a1), env_p(ep) {}
    value call(vector< value > args, shared_ptr< environment > caller_env_p)
    {
      check(args.size() == 1, 
            "wrong number of arguments to 'push-front! <expr>' (must be 1).");
      value& list_ref = apply_visitor(native_ref_visitor(env_p), arg1);
      value arg2 = eval(args.front(), caller_env_p);
      apply_visitor(push_front_visitor(), list_ref, arg2);
      return nil();
    }
  private:
    value arg1;
    shared_ptr< environment > env_p;
  };

  value push_front::call(vector< value > args, shared_ptr< environment > caller_env_p)
  {
    check(args.size() == 1 || args.size() == 2,
          "wrong number of arguments to 'push-front!' (must be 1 or 2).");
    value arg1 = args[0];
    if (args.size() == 1)
      return make_shared< push_front_partial >(arg1, caller_env_p);
    value& list_ref = apply_visitor(native_ref_visitor(caller_env_p), arg1);
    value arg2 = eval(args[1], caller_env_p);
    apply_visitor(push_front_visitor(), list_ref, arg2);
    return nil();
  }

  class push_back_visitor : public static_visitor<> {
  public:
    template< typename T>
    void operator()(list& lst, const T& val) const
    {
      lst.push_back(val);
    }
    template< typename T>
    void operator()(shared_ptr< reference >& lst_ref, const T& val) const
    {
      value& lst = lst_ref->get_native_ref();
      value v(val);
      apply_visitor(push_back_visitor(), lst, v);
    }
    template< typename T, typename U>
    void operator()(T& t, const U& u) const
    {
      check(false, 
            "arguments to 'push-back!' must be a reference to a list and a value.");
    }
  };

  class push_back_partial : public lambda {
  public:
    push_back_partial(value a1, shared_ptr< environment > ep) : arg1(a1), env_p(ep) {}
    value call(vector< value > args, shared_ptr< environment > caller_env_p)
    {
      check(args.size() == 1, 
            "wrong number of arguments to 'push-back! <expr>' (must be 1).");
      value& list_ref = apply_visitor(native_ref_visitor(env_p), arg1);
      value arg2 = eval(args.front(), caller_env_p);
      apply_visitor(push_back_visitor(), list_ref, arg2);
      return nil();
    }
  private:
    value arg1;
    shared_ptr< environment > env_p;
  };

  value push_back::call(vector< value > args, shared_ptr< environment > caller_env_p)
  {
    check(args.size() == 1 || args.size() == 2,
          "wrong number of arguments to 'push-back!' (must be 1 or 2).");
    value arg1 = args[0];
    if (args.size() == 1)
      return make_shared< push_back_partial >(arg1, caller_env_p);
    value& list_ref = apply_visitor(native_ref_visitor(caller_env_p), arg1);
    value arg2 = eval(args[1], caller_env_p);
    apply_visitor(push_back_visitor(), list_ref, arg2);
    return nil();
  }

  class pop_front_visitor : public static_visitor<> {
  public:
    void operator()(list& lst) const
    {
      lst.pop_front();
    }
    void operator()(shared_ptr< reference >& lst_ref) const
    {
      value& lst = lst_ref->get_native_ref();
      apply_visitor(pop_front_visitor(), lst);
    }
    template< typename T >
    void operator()(T& t) const
    {
      check(false, 
            "argument to 'pop-front!' must be a reference to a list.");
    }
  };

  value pop_front::call(vector< value > args, shared_ptr< environment > caller_env_p)
  {
    check(args.size() == 1, 
          "wrong number of arguments to 'pop-front!' (must be 1).");
    value arg1 = args[0];
    value& list_ref = apply_visitor(native_ref_visitor(caller_env_p), arg1);
    apply_visitor(pop_front_visitor(), list_ref);
    return nil();
  }

  class pop_back_visitor : public static_visitor<> {
  public:
    void operator()(list& lst) const
    {
      lst.pop_back();
    }
    void operator()(shared_ptr< reference >& lst_ref) const
    {
      value& lst = lst_ref->get_native_ref();
      apply_visitor(pop_back_visitor(), lst);
    }
    template< typename T >
    void operator()(T& t) const
    {
      check(false, 
            "argument to 'pop-back!' must be a reference to a list.");
    }
  };

  value pop_back::call(vector< value > args, shared_ptr< environment > caller_env_p)
  {
    check(args.size() == 1, 
          "wrong number of arguments to 'pop-back!' (must be 1).");
    value arg1 = args[0];
    value& list_ref = apply_visitor(native_ref_visitor(caller_env_p), arg1);
    apply_visitor(pop_back_visitor(), list_ref);
    return nil();
  }
  
  value delay::call(vector< value > args, shared_ptr< environment > caller_env_p)
  {
    check(args.size() == 1, "wrong number of arguments to 'delay' (must be 1).");
    return make_shared< delayed >(args.front(), caller_env_p);
  }

  class force_delayed_visitor : public static_visitor< value > {
  public:
    value operator()(const shared_ptr< delayed >& del) const
    {
      return del->force();
    }
    template< typename T>
    value operator()(const T& t) const
    {
      check(false, "argument to 'force' must be a delayed computation.");
    }
  };

  value force::call(vector< value > args, shared_ptr< environment > caller_env_p)
  {
    check(args.size() == 1, "wrong number of arguments to 'force' (must be 1).");
    value arg1(eval(args.front(), caller_env_p));
    return apply_visitor(force_delayed_visitor(), arg1);
  }

  value print::call(vector< value > args, shared_ptr< environment > caller_env_p)
  {
    check(args.size() == 1, "wrong number of arguments to 'print' (must be 1).");
    output(cout, eval(args[0], caller_env_p));
    return nil();
  }

  class print_string_visitor : public static_visitor<> {
  public:
    void operator()(const string& str) const
    {
      cout << unescape(str);
    }
    template< typename T>
    void operator()(const T& t) const
    {
      check(false, "argument to 'print-string' must be a string.");
    }
  };

  value print_string::call(vector< value > args, shared_ptr< environment > caller_env_p)
  {
    check(args.size() == 1, "wrong number of arguments to 'print-string' (must be 1).");
    value arg1 = eval(args.front(), caller_env_p);
    apply_visitor(print_string_visitor(), arg1);
    return nil();
  }

  value print_to_string::call(vector< value > args, 
                              shared_ptr< environment > caller_env_p)
  {
    check(args.size() == 1, 
          "wrong number of arguments to 'print-to-string' (must be 1).");
    stringstream iss;
    output(iss, eval(args[0], caller_env_p));
    return iss.str();
  }
  
  value read::call(vector< value > args, shared_ptr< environment > caller_env_p)
  {
    check(args.empty(), "'read' takes no arguments.");
    string input;
    getline(cin, input);
    return eval(parse(input), caller_env_p);
  } 
  
  value read_string::call(vector< value > args, shared_ptr< environment > caller_env_p)
  {
    check(args.empty(), "'read-string' takes no arguments.");
    string input;
    getline(cin, input);
    return escape(input);
  }

  class read_from_string_visitor : public static_visitor< value > {
  public:
    read_from_string_visitor(shared_ptr< environment > ep) : env_p(ep) {}
    value operator()(const string& str) const
    {
      return eval(parse(str), env_p);
    }
    template< typename T>
    value operator()(const T& t) const
    {
      check(false, "argument to 'read-from-string' must be a string.");
    }
  private:
    shared_ptr< environment > env_p;
  };

  value read_from_string::call(vector< value > args, 
                               shared_ptr< environment > caller_env_p)
  {
    check(args.size() == 1, 
          "wrong number of arguments to 'read-from-string' (must be 1).");
    value arg1 = eval(args.front(), caller_env_p);
    return apply_visitor(read_from_string_visitor(caller_env_p), arg1);
  }

  void add_builtins(shared_ptr< environment > env_p)
  {
    env_p->set("nil", nil());
    env_p->set("true", true);
    env_p->set("false", false);
    env_p->set("quote", make_shared< quote >());
    env_p->set("eval", make_shared< evaluate >());
    env_p->set("list", make_shared< make_list >());
    env_p->set("load", make_shared< load >());
    env_p->set("=", make_shared< equals >());
    env_p->set("<", make_shared< less_than >());
    env_p->set("+", make_shared< plus >());
    env_p->set("-", make_shared< minus >());
    env_p->set("*", make_shared< times >());
    env_p->set("/", make_shared< divide >());
    env_p->set("%", make_shared< modulo >());
    env_p->set("random", make_shared< random_int >());
    env_p->set("rand-max", RAND_MAX);
    env_p->set("atom?", make_shared< is_atom >());
    env_p->set("len", make_shared< len >());
    env_p->set("cons", make_shared< cons >());
    env_p->set("head", make_shared< head >());
    env_p->set("tail", make_shared< tail >());
    env_p->set("elem", make_shared< elem >());
    env_p->set("set-elem!", make_shared< set_elem >());
    env_p->set("push-front!", make_shared< push_front >());
    env_p->set("push-back!", make_shared< push_back >());
    env_p->set("pop-front!", make_shared< pop_front >());
    env_p->set("pop-back!", make_shared< pop_back >());
    env_p->set("delay", make_shared< delay >());
    env_p->set("force", make_shared< force >());
    env_p->set("print", make_shared< print >());
    env_p->set("print-string", make_shared< print_string >());
    env_p->set("print-to-string", make_shared< print_to_string >());
    env_p->set("read", make_shared< read >());
    env_p->set("read-string", make_shared< read_string >());
    env_p->set("read-from-string", make_shared< read_from_string >());
    srand(time(nullptr));
  }

} // namespace lime
