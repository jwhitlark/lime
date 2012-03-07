// STL headers
#include <iostream>

// lime headers
#include <core.hpp>
#include <eval.hpp>
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
        p = symbol(p.substr(1));
        check(p.size() > 0, "unnamed reference argument.");
      }
      else
        reference_arg.push_back(false);
      params.push_back(p);
    }
  }
  
  shared_ptr< reference > reference_visitor::operator()(const symbol& sym) const
  {
    check(env_p->find(sym), "symbol '" + sym + "' not found.");
    return make_shared< reference >(sym, env_p);
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
    return make_shared< lambda >(pars, ref_arg, expr, env_p);
  }

  bool stream::empty() const
  {
    return is_empty;
  }

  value stream::head() const
  {
    return head_val;
  }
  
  shared_ptr< stream > stream_visitor::operator()(const shared_ptr< stream >& s_p) const
  {
    return s_p;
  }
  
  template< typename T >
  shared_ptr< stream > stream_visitor::operator()(const T& t) const 
  {
    check(false, "expression does not evaluate to stream.");
  }
    
  shared_ptr< stream > stream::tail()
  {
    if (!tail_cache_p) {
      value tail_value = eval(tail_expr, creation_env_p);
      tail_cache_p = apply_visitor(stream_visitor(), tail_value);
    }                         
    return tail_cache_p;
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
    void operator()(const shared_ptr< stream >& str_p) const
    {
      if (!str_p->empty())
        out_stream << "(" << str_p->head() << " ...)";
      else
        out_stream << "()";
    }
    void operator()(const symbol& sym) const
    {
      out_stream << sym;
    }
    void operator()(const shared_ptr< lambda >& lam_p) const
    {
      out_stream << "lambda at address " << lam_p;
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
