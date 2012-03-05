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

  value lambda::call(vector< value > args, shared_ptr< environment > caller_env_p)
  {
    check(args.size() <= params.size(), "too many arguments to lambda.");
    check(args.size() > 0 || params.size() == 0, "lambda called without arguments.");
    auto local_env_p = nested_environment(creation_env_p);
    for (int i = 0; i < args.size(); ++i)
      local_env_p->set(params[i], eval(args[i], caller_env_p));
    if (args.size() < params.size())
      return partial(args.size(), local_env_p);
    return eval(expr, local_env_p);
  }

  shared_ptr< lambda > lambda::partial(int n_supplied_args, shared_ptr< environment > env_p)
  {
    vector< symbol > pars(begin(params) + n_supplied_args, end(params));
    return make_shared< lambda >(pars, expr, env_p);
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
    void operator()(const symbol& sym) const
    {
      out_stream << sym;
    }
    void operator()(const shared_ptr< lambda >& lam_p) const
    {
      out_stream << "lambda at address " << lam_p;
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

  shared_ptr< environment > nested_environment(shared_ptr< environment > outer_env_p)
  {
    auto nested_env_p = make_shared< environment >();
    nested_env_p->outer_env_p = outer_env_p;
    return nested_env_p;
  }

} // namespace lime
