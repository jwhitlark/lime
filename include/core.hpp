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

  class lambda;

  class stream;

  typedef variant< symbol, 
                   list, 
                   int,
                   string, 
                   bool, 
                   shared_ptr< lambda >,
                   shared_ptr< stream >,
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

  class environment;

  class lambda {
  public:
    lambda() {}
    lambda(vector< symbol > pars, value x, shared_ptr< environment > e) : 
      expr(x), creation_env_p(e), params(pars) {}
    virtual value call(vector< value > args, shared_ptr< environment > caller_env_p);
    shared_ptr< lambda > partial(int n_supplied_args, shared_ptr< environment > env_p);
  private:
    vector< symbol > params;
    value expr;
    shared_ptr< environment> creation_env_p;
  };

  class stream {
  public:
    stream() : is_empty(true) {}
    stream(value h, value t, shared_ptr< environment > ep) : 
      head_val(h), tail_expr(t), creation_env_p(ep), is_empty(false) {}
    bool empty() const;
    value head() const;
    shared_ptr< stream > tail();
  private:
    bool is_empty;
    value head_val, tail_expr;
    shared_ptr< environment > creation_env_p;
    shared_ptr< stream > tail_cache_p;
  };

  class stream_visitor : public static_visitor< shared_ptr< stream > > {
  public:
    shared_ptr< stream > operator()(const shared_ptr< stream >& s_p) const;
    template< typename T >
    shared_ptr< stream > operator()(const T& t) const;
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
    friend shared_ptr< environment > nested_environment(shared_ptr< environment > 
                                                        outer_env_p);
  private:
    shared_ptr< environment > outer_env_p;
    unordered_map< symbol, value, symbol_hash > values;
  };

  shared_ptr< environment > nested_environment(shared_ptr< environment > outer_env_p);

} // namespace lime

#endif // __CORE_HPP__
