// STL headers
#include <iostream>

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
  using std::make_shared;

  // Boost
  using boost::apply_visitor;
  using boost::static_visitor;

  // lime
  using lime::check;
  using lime::eval;
  using lime::nil;
  using lime::parse;
  using lime::print_stream;

  value quote::call(vector< value > args, shared_ptr< environment > caller_env_p)
  {
    check(args.size() == 1, "wrong number of arguments to 'quote' (must be 1).");
    return args.front();
  }

  value make_list::call(vector< value > args, shared_ptr< environment > caller_env_p)
  {
    list lst;
    for (value arg: args)
      lst.push_back(eval(arg, caller_env_p));
    return lst;
  }  

  class require_visitor : public static_visitor<> {
  public:
    require_visitor(shared_ptr< environment > ep) : env_p(ep) {}
    void operator()(const string& path) const
    {
      load_file(path, env_p);
    }
    template< typename T >
    void operator()(const T& t) const
    {
      check(false, "argument to 'require' must be a string.");
    }
  private:
    shared_ptr< environment > env_p;
  };

  value require::call(vector< value > args, shared_ptr< environment > caller_env_p)
  {
    check(args.size() == 1, "wrong number of arguments to 'require' (must be 1).");
    apply_visitor(require_visitor(caller_env_p), args.front());
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

  class logical_and_partial : public lambda {
  public:
    logical_and_partial(value a1) : boolean1(apply_visitor(bool_visitor(), a1)) {}
    value call(vector< value > args, shared_ptr< environment > caller_env_p)
    {
      check(args.size() == 1, "wrong number of arguments to 'and <expr>' (must be 1).");
      if (boolean1) {
        value arg2 = eval(args.front(), caller_env_p);
        return apply_visitor(bool_visitor(), arg2);
      }
      return false;
    }
  private:
    bool boolean1;
  };

  value logical_and::call(vector< value > args, shared_ptr< environment > caller_env_p)
  {
    check(args.size() == 1 || args.size() == 2, 
          "wrong number of arguments to 'and' (must be 1 or 2).");
    value arg1 = eval(args[0], caller_env_p);
    if (args.size() == 1)
      return make_shared< logical_and_partial >(arg1);
    bool boolean1 = apply_visitor(bool_visitor(), arg1);
    if (boolean1) {
      value arg2 = eval(args.front(), caller_env_p);
      return apply_visitor(bool_visitor(), arg2);
    }
    return false;
  }

  class logical_or_partial : public lambda {
  public:
    logical_or_partial(value a1) : boolean1(apply_visitor(bool_visitor(), a1)) {}
    value call(vector< value > args, shared_ptr< environment > caller_env_p)
    {
      check(args.size() == 1, "wrong number of arguments to 'and <expr>' (must be 1).");
      if (boolean1)
        return true;
      value arg2 = eval(args.front(), caller_env_p);
      return apply_visitor(bool_visitor(), arg2);
    }
  private:
    bool boolean1;
  };

  value logical_or::call(vector< value > args, shared_ptr< environment > caller_env_p)
  {
    check(args.size() == 1 || args.size() == 2, 
          "wrong number of arguments to 'and' (must be 1 or 2).");
    value arg1 = eval(args[0], caller_env_p);
    if (args.size() == 1)
      return make_shared< logical_or_partial >(arg1);
    bool boolean1 = apply_visitor(bool_visitor(), arg1);
      if (boolean1)
        return true;
      value arg2 = eval(args.front(), caller_env_p);
      return apply_visitor(bool_visitor(), arg2);
  }

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

  class is_null_visitor : public static_visitor< bool > {
  public:
    bool operator()(const list& lst) const
    {
      return lst.empty();
    }
    template< typename T >
    bool operator()(const T& t) const
    {
      return false;
    }
  };

  value is_null::call(vector< value > args, shared_ptr< environment > caller_env_p)
  {
    check(args.size() == 1, "wrong number of arguments to 'null?' (must be 1).");
    value arg = eval(args.front(), caller_env_p);
    return apply_visitor(is_null_visitor(), arg);
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
    value operator()(const list& lst, const int i) const
    {
      check(i >= 1 && i <= lst.size(), "list index out of range.");
      return lst[i - 1];
    }
    template< typename T, typename U >
    value operator()(const T& t, const U& u) const
    {
      check(false, "arguments to 'elem' must be a non-empty list and an integer.");
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
  
  class memoized_proc : public lambda {
  public:
    memoized_proc(value x, shared_ptr< environment > e) : 
      lambda(vector< symbol >(), x, e),
      already_run(false) {}
    value call(vector< value > args, shared_ptr< environment > caller_env_p)
    {
      if (!already_run)
        cache = lambda::call(args, caller_env_p);
      return cache;
    }
  private:
    bool already_run;
    value cache;
  };

  value delay::call(vector< value > args, shared_ptr< environment > caller_env_p)
  {
    check(args.size() == 1, "wrong number of arguments to 'delay' (must be 1).");
    return make_shared< memoized_proc >(args[0], caller_env_p);
  }

  class cons_stream_partial : public lambda {
  public:
    cons_stream_partial(value a1) : arg1(a1) {}
    value call(vector< value > args, shared_ptr< environment > caller_env_p)
    {
      check(args.size() == 1, "wrong number of arguments to 'cons-stream <expr>' (must be 1).");
      value arg2 = delay().call(vector< value > { args[0] }, caller_env_p);
      list stream(deque< value > { arg1, arg2 });
      return stream;
    }
  private:
    value arg1;
  };

  value cons_stream::call(vector< value > args, shared_ptr< environment > caller_env_p)
  {
    check(args.size() == 1 || args.size() == 2, 
          "wrong number of arguments to 'cons-stream' (must be 1 or 2).");
    value arg1 = eval(args[0], caller_env_p);
    if (args.size() == 1)
      return make_shared< cons_stream_partial >(arg1);
    value arg2 = delay().call(vector< value > { args[1] }, caller_env_p);
    list stream(deque< value > { arg1, arg2 });
    return stream;
  }

  value print::call(vector< value > args, shared_ptr< environment > caller_env_p)
  {
    check(args.size() == 1, "wrong number of arguments to 'print' (must be 1).");
    print_stream(cout, eval(args[0], caller_env_p));
    return nil();
  }

  value read::call(vector< value > args, shared_ptr< environment > caller_env_p)
  {
    check(args.empty(), "'read' takes no arguments.");
    string input;
    getline(cin, input);
    return eval(parse(input), caller_env_p);
  }

  void add_builtins(shared_ptr< environment > env_p)
  {
    env_p->set("nil", nil());
    env_p->set("true", true);
    env_p->set("false", false);
    env_p->set("quote", make_shared< quote >());
    env_p->set("list", make_shared< make_list >());
    env_p->set("require", make_shared< require >());
    env_p->set("=", make_shared< equals >());
    env_p->set("<", make_shared< less_than >());
    env_p->set("+", make_shared< plus >());
    env_p->set("-", make_shared< minus >());
    env_p->set("*", make_shared< times >());
    env_p->set("/", make_shared< divide >());
    env_p->set("%", make_shared< modulo >());
    env_p->set("and", make_shared< logical_and >());
    env_p->set("or", make_shared< logical_or >());
    env_p->set("atom?", make_shared< is_atom >());
    env_p->set("null?", make_shared< is_null >());
    env_p->set("cons", make_shared< cons >());
    env_p->set("head", make_shared< head >());
    env_p->set("tail", make_shared< tail >());
    env_p->set("elem", make_shared< elem >());
    env_p->set("delay", make_shared< delay >());
    env_p->set("cons-stream", make_shared< cons_stream >());
    env_p->set("print", make_shared< print >());
    env_p->set("read", make_shared< read >());
  }

} // namespace lime
