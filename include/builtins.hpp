#ifndef __BUILTINS_HPP__
#define __BUILTINS_HPP__

// lime headers
#include <core.hpp>

namespace lime {
  // STL
  using std::shared_ptr;

  // lime
  using lime::environment;
  using lime::lambda;
  using lime::value;

  class quote : public lambda {
  public:
    value call(vector< value > args, shared_ptr< environment > caller_env_p);
  };

  class make_list : public lambda {
  public:
    value call(vector< value > args, shared_ptr< environment > caller_env_p);
  };

  class require : public lambda {
  public:
    value call(vector< value > args, shared_ptr< environment > caller_env_p);
  };

  class equals : public lambda {
  public:
    value call(vector< value > args, shared_ptr< environment > caller_env_p);
  };

  class less_than : public lambda {
  public:
    value call(vector< value > args, shared_ptr< environment > caller_env_p);
  };

  class plus : public lambda {
  public:
    value call(vector< value > args, shared_ptr< environment > caller_env_p);
  };

  class minus : public lambda {
  public:
    value call(vector< value > args, shared_ptr< environment > caller_env_p);
  };

  class times : public lambda {
  public:
    value call(vector< value > args, shared_ptr< environment > caller_env_p);
  };

  class divide : public lambda {
  public:
    value call(vector< value > args, shared_ptr< environment > caller_env_p);
  };

  class modulo : public lambda {
  public:
    value call(vector< value > args, shared_ptr< environment > caller_env_p);
  };

  class is_atom : public lambda {
  public:
    value call(vector< value > args, shared_ptr< environment > caller_env_p);
  };

  class is_null : public lambda {
  public:
    value call(vector< value > args, shared_ptr< environment > caller_env_p);
  };

  class cons : public lambda {
  public:
    value call(vector< value > args, shared_ptr< environment > caller_env_p);
  };

  class head : public lambda {
  public:
    value call(vector< value > args, shared_ptr< environment > caller_env_p);
  };

  class tail : public lambda {
  public:
    value call(vector< value > args, shared_ptr< environment > caller_env_p);
  };

  class elem : public lambda {
  public:
    value call(vector< value > args, shared_ptr< environment > caller_env_p);
  };

  class delay : public lambda {
  public:
    value call(vector< value > args, shared_ptr< environment > caller_env_p);
  };

  class cons_stream : public lambda {
  public:
    value call(vector< value > args, shared_ptr< environment > caller_env_p);
  };
  
  class print : public lambda {
  public:
    value call(vector< value > args, shared_ptr< environment > caller_env_p);
  };
  
  class read : public lambda {
  public:
    value call(vector< value > args, shared_ptr< environment > caller_env_p);
  };

  void add_builtins(shared_ptr< environment > env_p);

} // namespace lime

#endif // __BUILTINS_HPP__
