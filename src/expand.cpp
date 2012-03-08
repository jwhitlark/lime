// lime headers
#include <expand.hpp>

namespace lime {
  
  class expand_visitor : public static_visitor< value > {
    // TODO
  };

  value expand(value expr, vector< value > params, vector< value > args)
  {
    return apply_visitor(expand_visitor(params, args), expr);
  }

} // namespace lime
