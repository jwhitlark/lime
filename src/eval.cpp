// STL headers
#include <algorithm>

// lime headers
#include <eval.hpp>
#include <interpreter.hpp>

namespace lime {
  // STL
  using std::make_shared;
  using std::transform;

  // Boost
  using boost::apply_visitor;
  using boost::static_visitor;

  // lime
  using lime::check;
  using lime::load_file;
  using lime::nested_environment;

  class test_visitor : public static_visitor< bool > {
  public:
    bool operator()(bool b) const
    {
      return b;
    }
    template< typename T >
    bool operator()(const T& t) const
    {
      check(false, "first argument to 'if' must evaluate to boolean.");
    }
  };

  class parameter_visitor : public static_visitor< symbol > {
  public:
    symbol operator()(const symbol& sym) const
    {
      return sym;
    }
    template< typename T >
    symbol operator()(const T& t) const
    {
      check(false, "parameter-list in lambda definition must only contain symbols.");
    }
  };

  class lambda_params_visitor : public static_visitor< vector< symbol > > {
  public:
    vector< symbol > operator()(const list& lst) const
    {
      vector< symbol > params(lst.size());
      transform(begin(lst), end(lst), begin(params), [](value v) {
          return apply_visitor(parameter_visitor(), v);
        });
      return params;
    }
    template< typename T >
    vector< symbol > operator()(const T& t) const
    {
      check(false, "first argument to 'lambda' must be a list of parameters.");
    }
  };

  class function_name_visitor : public static_visitor< symbol > {
  public:
    symbol operator()(const symbol& sym) const
    {
      return sym;
    }
    template< typename T >
    symbol operator()(const T& t) const
    {
      check(false, "function name must be a symbol.");
    }
  };

  class define_visitor : public static_visitor<> {
  public:
    define_visitor(list x, shared_ptr< environment > ep) : expr(x), env_p(ep) {}
    void operator()(const symbol& sym) const
    {
      check(!env_p->find_local(sym), "attempting to redefine symbol '" + sym + "'.");
      env_p->set(sym, eval(expr[2], env_p));
    }
    void operator()(const list& lst) const
    {
      check(lst.size() >= 0, "syntax error in 'define'.");
      value sym_v = lst.head();
      symbol sym = apply_visitor(function_name_visitor(), sym_v);
      check(!env_p->find_local(sym), "attempting to redefine symbol '" + sym + "'.");
      value params_v = lst.tail();
      vector< symbol > params = apply_visitor(lambda_params_visitor(), params_v);
      env_p->set(sym, make_shared< lambda >(params, expr[2], env_p));
    }
    template< typename T >
    void operator()(const T& t) const
    {
      check(false, "first argument to 'define' must be a symbol.");
    }
  private:
    list expr;
    shared_ptr< environment > env_p;
  };

  class set_ref_visitor : public static_visitor< bool > {
  public:
    set_ref_visitor(list x, shared_ptr< environment > ep) : expr(x), env_p(ep) {}
    bool operator()(const shared_ptr< reference >& ref) const
    {
      ref->set(eval(expr[2], env_p));
      return true;
    }
    template< typename T>
    bool operator()(const T& t) const
    {
      return false;
    }
  private:
    list expr;
    shared_ptr< environment > env_p;
  };

  class set_visitor : public static_visitor<> {
  public:
    set_visitor(list x, shared_ptr< environment > ep) : expr(x), env_p(ep) {}
    void operator()(const symbol& sym) const
    {
      check(env_p->find(sym), "argument '" + sym + "' to 'set!' is undefined.");
      value val = env_p->get(sym);
      if (apply_visitor(set_ref_visitor(expr, env_p), val))
        return;
      if (env_p->find_local(sym))
        env_p->set(sym, eval(expr[2], env_p));
      else
        env_p->set_outermost(sym, eval(expr[2], env_p));
    }
    template< typename T >
    void operator()(const T& t) const
    {
      check(false, "first argument to 'set!' must be a symbol.");
    }
  private:
    list expr;
    shared_ptr< environment > env_p;
  };

  class lambda_call_visitor : public static_visitor< value > {
  public:
    lambda_call_visitor(list x, shared_ptr< environment > ep) : expr(x), env_p(ep) {}
    value operator()(const shared_ptr< lambda >& lam_p) const
    {
      vector< value > args(begin(expr) + 1, end(expr));
      return lam_p->call(args, env_p);
    }
    template< typename T>
    value operator()(const T& t) const
    {
      check(false, "first element of a list must be a lambda or builtin operator.");
    }
  private:
    list expr;
    shared_ptr< environment > env_p;
  };

  class operator_visitor : public static_visitor< value > {
  public:
    operator_visitor(list x, shared_ptr< environment > ep) : expr(x), env_p(ep) {}    
    value operator()(const symbol& sym) const
    {
      if (sym == "if") {
        check(expr.size() == 4, "wrong number of arguments to 'if' (must be 3).");
        test_visitor visitor;
        value condition = eval(expr[1], env_p);
        bool test = apply_visitor(visitor, condition);
        return eval(test ? expr[2] : expr[3], env_p);
      }
      else if (sym == "define") {
        check(expr.size() == 3, "wrong number of arguments to 'define' (must be 2).");
        apply_visitor(define_visitor(expr, env_p), expr[1]);
      }
      else if (sym == "set!") {
        check(expr.size() == 3, "wrong number of arguments to 'set!' (must be 2).");
        apply_visitor(set_visitor(expr, env_p), expr[1]);
      }
      else if (sym == "begin") {
        auto inner_env_p = nested_environment(env_p);
        for (int i = 1; i + 1 < expr.size(); ++i)
          eval(expr[i], inner_env_p);
        if (expr.size() > 1)
          return eval(expr.back(), inner_env_p);
        else
          return nil();
      }
      else if (sym == "lambda") {
        check(expr.size() == 3, "wrong number of arguments to 'lambda' (must be 2).");
        vector< symbol > params = apply_visitor(lambda_params_visitor(), expr[1]);
        return make_shared< lambda >(params, expr[2], env_p);
      }
      else { // sym must refer to a lambda
        lambda_call_visitor visitor(expr, env_p);
        value lam_v = eval(value(sym), env_p);
        return apply_visitor(visitor, lam_v);
      }
      return nil();
    }
    value operator()(const list& lambda_lst) const
    {
      value lam_p = eval(lambda_lst, env_p);
      return apply_visitor(lambda_call_visitor(expr, env_p), lam_p);
    }
    value operator()(const shared_ptr< reference >& ref) const
    {
      return operator()(ref->get());
    }
    template< typename T >
    value operator()(const T& t) const
    {
      check(false, "first element of a list must be a lambda or builtin operator.");
    }
  private:
    list expr;
    shared_ptr< environment > env_p;
  };
  
  class eval_visitor : public static_visitor< value > {
  public:
    eval_visitor(shared_ptr< environment > ep) : env_p(ep) {}
    value operator()(const symbol& sym) const
    {
      check(env_p->find(sym), "symbol '" + sym + "' not found.");
      return env_p->get(sym);
    }
    value operator()(const list& lst) const
    {
      return apply_visitor(operator_visitor(lst, env_p), lst.front());
    }
    value operator()(const shared_ptr< reference >& ref) const
    {
      return ref->get();
    }
    template< typename T >
    value operator()(const T& t) const
    {
      return t;
    }    
  private:
    shared_ptr< environment > env_p;
  };
  
  value eval(value expr, shared_ptr< environment > env_p)
  {
    return apply_visitor(eval_visitor(env_p), expr);
  }

} // namespace lime
