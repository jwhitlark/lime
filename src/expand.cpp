// STL headers
#include <unordered_map>

// lime headers
#include <expand.hpp>

namespace lime {
  // STL
  using std::unordered_map;  

  // lime
  using lime::symbol_hash;

  class expand_visitor : public static_visitor< value > {
  public:
    expand_visitor(vector< symbol > params, vector< value > args)
    {
      for (int i = 0; i < params.size(); ++i)
        substitutions[params[i]] = args[i];
    }
    expand_visitor(unordered_map< symbol, value, symbol_hash > sub)
      : substitutions(sub) {}
    value operator()(const symbol& sym)
    {
      if (substitutions.find(sym) != substitutions.end())
        return substitutions[sym];
      return sym;
    }
    value operator()(const list& lst) const
    {
      list new_lst;
      for (value expr: lst) {
        auto visitor = expand_visitor(substitutions);
        value new_expr(apply_visitor(visitor, expr));
        new_lst.push_back(new_expr);
      }
      return new_lst;
    }
    template< typename T>
    value operator()(const T& t) const
    {
      return t;
    }
  private:
    unordered_map< symbol, value, symbol_hash > substitutions;
  };

  value expand(value expr, vector< symbol > params, vector< value > args)
  {
    return apply_visitor(expand_visitor(params, args), expr);
  }

} // namespace lime
