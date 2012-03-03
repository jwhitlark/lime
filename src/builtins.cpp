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
    template< typename T >
    bool operator()(const T& a, const T& b) const
    {
      check(false, "type of arguments to '=' not supported.");
    }
    template< typename T, typename U >
    bool operator()(const T& a, const U& b) const
    {
      check(false, "arguments to '=' must be of the same type.");
    }
  };

  value equals::call(vector< value > args, shared_ptr< environment > caller_env_p)
  {
    check(args.size() == 2, "wrong number of arguments to '=' (must be 2).");
    value arg1 = eval(args[0], caller_env_p);
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

  value less_than::call(vector< value > args, shared_ptr< environment > caller_env_p)
  {
    check(args.size() == 2, "wrong number of arguments to '<' (must be 2).");
    value arg1 = eval(args[0], caller_env_p);
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

  value plus::call(vector< value > args, shared_ptr< environment > caller_env_p)
  {
    check(args.size() == 2, "wrong number of arguments to '+' (must be 2).");
    value arg1 = eval(args[0], caller_env_p);
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

  value minus::call(vector< value > args, shared_ptr< environment > caller_env_p)
  {
    check(args.size() == 2, "wrong number of arguments to '-' (must be 2).");
    value arg1 = eval(args[0], caller_env_p);
    value arg2 = eval(args[1], caller_env_p);
    return apply_visitor(minus_visitor(), arg1, arg2);
  }

  class times_visitor : public static_visitor< value > {
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

  value times::call(vector< value > args, shared_ptr< environment > caller_env_p)
  {
    check(args.size() == 2, "wrong number of arguments to '*' (must be 2).");
    value arg1 = eval(args[0], caller_env_p);
    value arg2 = eval(args[1], caller_env_p);
    return apply_visitor(times_visitor(), arg1, arg2);
  }

  class divide_visitor : public static_visitor< value > {
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

  value divide::call(vector< value > args, shared_ptr< environment > caller_env_p)
  {
    check(args.size() == 2, "wrong number of arguments to '/' (must be 2).");
    value arg1 = eval(args[0], caller_env_p);
    value arg2 = eval(args[1], caller_env_p);
    return apply_visitor(divide_visitor(), arg1, arg2);
  }

  class modulo_visitor : public static_visitor< value > {
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

  value modulo::call(vector< value > args, shared_ptr< environment > caller_env_p)
  {
    check(args.size() == 2, "wrong number of arguments to '%' (must be 2).");
    value arg1 = eval(args[0], caller_env_p);
    value arg2 = eval(args[1], caller_env_p);
    return apply_visitor(modulo_visitor(), arg1, arg2);
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

  value cons::call(vector< value > args, shared_ptr< environment > caller_env_p)
  {
    check(args.size() == 2, "wrong number of arguments to 'cons' (must be 2).");
    value arg1 = eval(args[0], caller_env_p);
    value arg2 = eval(args[1], caller_env_p);
    return apply_visitor(cons_visitor(), arg1, arg2);
  }

  class head_visitor : public static_visitor< value > {
  public:
    value operator()(const list& lst) const
    {
      check(!lst.empty(), "argument to 'head' must be a non-empty list.");
      return lst.front();
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
      return deque< value >(begin(lst) + 1, end(lst));
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
    env_p->set("=", make_shared< equals >());
    env_p->set("<", make_shared< less_than >());
    env_p->set("+", make_shared< plus >());
    env_p->set("-", make_shared< minus >());
    env_p->set("*", make_shared< times >());
    env_p->set("/", make_shared< divide >());
    env_p->set("%", make_shared< modulo >());
    env_p->set("atom?", make_shared< is_atom >());
    env_p->set("null?", make_shared< is_null >());
    env_p->set("cons", make_shared< cons >());
    env_p->set("head", make_shared< head >());
    env_p->set("tail", make_shared< tail >());
    env_p->set("print", make_shared< print >());
    env_p->set("read", make_shared< read >());
  }

} // namespace lime
