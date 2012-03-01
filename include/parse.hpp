#ifndef __PARSE_HPP__
#define __PARSE_HPP__

// STL headers
#include <string>
#include <vector>

// lime headers
#include <core.hpp>

namespace lime {
  // STL
  using std::string;
  using std::vector;

  // lime
  using lime::value;

  value parse(const string& code);

  vector< string > split(const string& code);

  bool paren_match(const string& code);

} // namespace lime

#endif // __PARSE_HPP__
