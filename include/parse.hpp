#ifndef __PARSE_HPP__
#define __PARSE_HPP__

// STL headers
#include <stack>
#include <string>
#include <vector>

// lime headers
#include <core.hpp>

namespace lime {
  // STL
  using std::stack;
  using std::string;
  using std::vector;

  // lime
  using lime::value;

  value parse(const string& code);

  vector< string > split(const string& code);

  string escape(const string& str);

  string unescape(const string& str);

  bool paren_match(const string& code);

  bool quot_match(const string& code);

  int indent(const string& code, stack< int >& open_parens, int indent_length=2);

} // namespace lime

#endif // __PARSE_HPP__
