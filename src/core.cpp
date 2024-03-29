// STL headers
#include <iostream>

// lime headers
#include <core.hpp>
#include <eval.hpp>
#include <expand.hpp>
#include <interpreter.hpp>
#include <parse.hpp>

namespace lime {
  // STL
  using std::cout;
  using std::make_shared;

  // Boost
  using boost::apply_visitor;
  using boost::static_visitor;

  // lime
  using lime::check;
  using lime::escape;
  using lime::eval;
  using lime::expand;

  value list::head() const
  {
    return front();
  }

  list list::tail() const
  {
    return deque< value >(begin() + 1, end());
  }

  value reference::get() const
  {
    check(env_p->find(sym), "reference to '" + sym + "' undefined.");
    return env_p->get(sym);
  }

  void reference::set(value val)
  {
    check(env_p->find(sym), "reference to '" + sym + "' undefined.");
    if (env_p->find_local(sym))
      env_p->set(sym, val);
    else
      env_p->set_outermost(sym, val);
  }

  value& reference::get_native_ref() const
  {
    check(env_p->find(sym), "reference to '" + sym + "' undefined.");
    return env_p->get_ref(sym);
  }

  lambda::lambda(vector< symbol > pars, value x, shared_ptr< environment > e) : 
    expr(x), creation_env_p(e)
  {
    for (symbol p: pars) {
      if (p.front() == '&') {
        reference_arg.push_back(true);
        delayed_arg.push_back(false);
        p = symbol(p.substr(1));
        check(p.size() > 0, "unnamed reference argument.");
      }
      else if (p.front() == '$') {
        reference_arg.push_back(false);
        delayed_arg.push_back(true);
        p = symbol(p.substr(1));
        check(p.size() > 0, "unnamed delayed argument.");
      }
      else {
        reference_arg.push_back(false);
        delayed_arg.push_back(false);
      }
      params.push_back(p);
    }
  }
  
  class make_reference_visitor : public static_visitor< shared_ptr< reference > > {
  public:
    make_reference_visitor(symbol s, shared_ptr< environment > ep) : sym(s), env_p(ep) {}
    shared_ptr< reference > operator()(const shared_ptr< reference >& ref) const
    {
      return ref;
    }
    template< typename T>
    shared_ptr< reference > operator()(const T& val) const
    {
      return make_shared< reference >(sym, env_p);
    }
  private:
    symbol sym;
    shared_ptr< environment > env_p;
  };

  shared_ptr< reference > reference_visitor::operator()(const symbol& sym) const
  {
    check(env_p->find(sym), "symbol '" + sym + "' not found.");
    value val(env_p->get(sym));
    return apply_visitor(make_reference_visitor(sym, env_p), val);
  }
  
  template< typename T>
  shared_ptr< reference > reference_visitor::operator()(const T& t) const
  {
    check(false, "attempting to get reference to non-symbol.");
  }

  value lambda::call(vector< value > args, shared_ptr< environment > caller_env_p)
  {
    check(args.size() <= params.size(), "too many arguments to lambda.");
    check(args.size() > 0 || params.size() == 0, "lambda called without arguments.");
    auto local_env_p = nested_environment(creation_env_p);
    for (int i = 0; i < args.size(); ++i)
      if (reference_arg[i])
        local_env_p->set(params[i],
                         apply_visitor(reference_visitor(caller_env_p), args[i]));
      else if (delayed_arg[i])
        local_env_p->set(params[i], make_shared< delayed >(args[i], caller_env_p));
      else
        local_env_p->set(params[i], eval(args[i], caller_env_p));
    if (args.size() < params.size())
      return partial(args.size(), local_env_p);
    return eval(expr, local_env_p);
  }

  shared_ptr< lambda > lambda::partial(int n_supplied_args, shared_ptr< environment > env_p)
  {
    vector< symbol > pars(begin(params) + n_supplied_args, end(params));
    vector< bool > ref_arg(begin(reference_arg) + n_supplied_args, end(reference_arg));
    vector< bool > del_arg(begin(delayed_arg) + n_supplied_args, end(delayed_arg));
    return make_shared< lambda >(pars, ref_arg, del_arg, expr, env_p);
  }
  
  value macro::call(vector< value > args, shared_ptr< environment > caller_env_p)
  {
    check(args.size() == params.size(), "wrong number of arguments to macro.");
    value expanded_expr(expand(expr, params, args));
    return eval(expanded_expr, caller_env_p);
  }

  value delayed::force()
  {
    if (!already_run) {
      cache = eval(expr, env_p);
      already_run = true;
    }
    return cache;
  }

  class output_visitor : public static_visitor<> {
  public:
    output_visitor(ostream& out) : out_stream(out) {}
    void operator()(int i) const
    {
      out_stream << i;
    }
    void operator()(const string& s) const
    {
      out_stream << '"' << escape(s) << '"';
    }
    void operator()(bool b) const
    {
      out_stream << (b ? "true" : "false");
    }
    void operator()(const nil& n) const
    {
      out_stream << "nil";
    }
    void operator()(const list& l) const
    {
      out_stream << "(";
      for (int i = 0; i + 1 < l.size(); ++i)
        out_stream << l[i] << " ";
      if (!l.empty())
        out_stream << l.back();
      out_stream << ")";
    }
    void operator()(const shared_ptr< delayed >& del) const
    {
      out_stream << "...";
    }
    void operator()(const symbol& sym) const
    {
      out_stream << sym;
    }
    void operator()(const shared_ptr< lambda >& lam_p) const
    {
      out_stream << "lambda at address " << lam_p;
    }
    void operator()(const shared_ptr< macro >& mac_p) const
    {
      out_stream << "macro at address " << mac_p;
    }
    void operator()(const shared_ptr< reference >& ref) const
    {
      out_stream << ref->get();
    }
  private:
    ostream& out_stream;
  };

  ostream& operator<<(ostream& out_stream, const value& val)
  {
    output(out_stream, val);
  }

  ostream& output(ostream& out_stream, const value& val)
  {
    apply_visitor(output_visitor(out_stream), val);
    return out_stream;
  }

  bool environment::find(symbol sym)
  {
    return (values.find(sym) != end(values) || 
            (outer_env_p && outer_env_p->find(sym)));
  }

  bool environment::find(string str)
  {
    return find(symbol(str));
  }
  
  value environment::get(symbol sym)
  {
    if (values.find(sym) != end(values))
      return values[sym];
    return outer_env_p->get(sym);
  }

  value environment::get(string str)
  {
    return get(symbol(str));
  }

  void environment::set(symbol sym, value val)
  {
    values[sym] = val;
  }

  void environment::set(string str, value val)
  {
    set(symbol(str), val);
  }

  bool environment::find_local(symbol sym)
  {
    return values.find(sym) != end(values);
  }

  void environment::set_outermost(symbol sym, value val)
  {
    if (outer_env_p && outer_env_p->find(sym))
      outer_env_p->set_outermost(sym, val);
    else
      set(sym, val);
  }  

  value& environment::get_ref(symbol sym)
  {
    if (values.find(sym) != end(values))
      return values[sym];
    return outer_env_p->get_ref(sym);
  }

  shared_ptr< environment > nested_environment(shared_ptr< environment > outer_env_p)
  {
    auto nested_env_p = make_shared< environment >();
    nested_env_p->outer_env_p = outer_env_p;
    return nested_env_p;
  }

} // namespace lime
