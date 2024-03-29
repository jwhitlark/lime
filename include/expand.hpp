#ifndef __EXPAND_HPP__
#define __EXPAND_HPP__

// lime headers
#include <core.hpp>

namespace lime {
  // lime
  using lime::symbol;
  using lime::value;

  value expand(value expr, vector< symbol > params, vector< value > args); 

} // namespace lime

#endif // __EXPAND_HPP__
