#ifndef __EVAL_HPP__
#define __EVAL_HPP__

// STL headers
#include <memory>

// lime headers
#include <core.hpp>

namespace lime {
  // STL
  using std::shared_ptr;

  // lime
  using lime::environment;
  using lime::value;

  value eval(value expr, shared_ptr< environment > env_p); 

} // namespace lime

#endif // __EVAL_HPP__
