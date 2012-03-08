#ifndef __CORE_HPP__
#define __CORE_HPP__

// STL headers
#include <deque>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

// Boost headers
#include <boost/variant.hpp>

namespace lime {
  // STL
  using std::basic_string;
  using std::deque;
  using std::hash;
  using std::ostream;
  using std::shared_ptr;
  using std::string;
  using std::unordered_map;
  using std::vector;

  // Boost
  using boost::static_visitor;
  using boost::variant;

  class symbol : public basic_string< char > {
  public:
    symbol() {}
    explicit symbol(const string& str) : basic_string< char >(str) {}
  };

  class symbol_hash {
  public:
    size_t operator()(const symbol& sym) const noexcept
    {
      return hash< basic_string< char > >()(sym);
    }
  };
  
  class nil {};

  class list;

  class reference;

  class lambda;

  class macro;

  class delayed;

  class environment;

  typedef variant< symbol, 
                   list, 
                   int,
                   string, 
                   bool,
                   shared_ptr< reference >,
                   shared_ptr< lambda >,
                   shared_ptr< macro >,
                   shared_ptr< delayed >,
                   nil > value;

  class list : public deque< value > {
  public:    
    list() {}
    list(deque< value > d) : deque< value >(d) {}
    list(value h, list t) : deque< value >(t)
    {
      push_front(h);
    }
    value head() const;
    list tail() const;
  };

  class reference {
  public:
    explicit reference(const symbol& s, shared_ptr< environment > ep) 
      : sym(s), env_p(ep) {}
    value get() const;
    void set(value val);
    value& get_native_ref() const;
  private: 
    symbol sym;
    shared_ptr< environment > env_p;
  };

  class reference_visitor : public static_visitor< shared_ptr< reference > > {
  public:
    reference_visitor(shared_ptr< environment > ep) : env_p(ep) {}
    shared_ptr< reference > operator()(const symbol& sym) const;
    template< typename T>
    shared_ptr< reference > operator()(const T& t) const;
  private:
    shared_ptr< environment > env_p;
  };

  class lambda {
  public:
    lambda() {}
    lambda(vector< symbol > pars, vector< bool > ref_arg, vector< bool > del_arg,
           value x, shared_ptr< environment > e) 
      : params(pars), reference_arg(ref_arg), delayed_arg(del_arg), expr(x), 
        creation_env_p(e) {}
    lambda(vector< symbol > pars, value x, shared_ptr< environment > e);
    virtual value call(vector< value > args, shared_ptr< environment > caller_env_p);
    shared_ptr< lambda > partial(int n_supplied_args, shared_ptr< environment > env_p);
  private:
    vector< symbol > params;
    vector< bool > reference_arg, delayed_arg;
    value expr;
    shared_ptr< environment> creation_env_p;
  };

  class macro {
  public:
    macro(vector< symbol > pars, value x) : params(pars), expr(x) {}
    value call(vector< value > args, shared_ptr< environment > caller_env_p);
  private:
    vector< symbol > params;
    value expr;
  };

  class delayed {
  public:
    delayed(value x, shared_ptr< environment > ep) : expr(x), env_p(ep),
                                                     already_run(false) {}
    value force();
  private:
    value expr;
    shared_ptr< environment > env_p;
    bool already_run;
    value cache;
  };

  ostream& operator<<(ostream& out_stream, const value& val);
  ostream& output(ostream& out_stream, const value& val);

  class environment {
  public:
    bool find(symbol sym);
    bool find(string str);
    value get(symbol sym);
    value get(string str);
    void set(symbol sym, value val);
    void set(string str, value val);
    bool find_local(symbol sym);
    void set_outermost(symbol sym, value val);
    value& get_ref(symbol sym);
    friend shared_ptr< environment > nested_environment(shared_ptr< environment > 
                                                        outer_env_p);
  protected:
    shared_ptr< environment > outer_env_p;
    unordered_map< symbol, value, symbol_hash > values;
  };

  shared_ptr< environment > nested_environment(shared_ptr< environment > outer_env_p);

} // namespace lime

#endif // __CORE_HPP__
