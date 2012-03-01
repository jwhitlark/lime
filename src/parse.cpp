// STL headers
#include <algorithm>
#include <deque>
#include <iostream>
#include <iterator>
#include <sstream>

// lime headers
#include <interpreter.hpp>
#include <parse.hpp>

namespace lime {
  // STL
  using std::back_inserter;
  using std::copy;
  using std::deque;
  using std::istream_iterator;
  using std::istringstream;

  // lime
  using lime::check;
  using lime::list;
  using lime::symbol;

  string add_blanks(const string& code)
  {
    string new_code;
    for (int i = 0; i < code.length(); ++i)
      if (code[i] == '(')
        new_code += " ( ";
      else if (code[i] == ')')
        new_code += " ) ";
      else
        new_code.push_back(code[i]);
    return new_code;
  }

  deque< string > tokenize(const string& code)
  {
    istringstream iss(add_blanks(code));
    deque< string > tokens;
    copy(istream_iterator< string >(iss),
         istream_iterator< string >(),
         back_inserter< deque< string > >(tokens));
    return tokens;
  }

  string unescape(const string& str)
  {
    string unescaped;
    for (int i = 0; i < str.length() - 1; ++i)
      if (str[i] == '\\' && str[i + 1] == 'n') {
        unescaped.push_back('\n');
        ++i;
      }
      else
        unescaped.push_back(str[i]);
    return unescaped;
  }

  value atom(const string& token)
  {
    check(token.length() > 0, "attempting to parse an empty token.");
    istringstream iss(token);
    int n;
    if (iss >> n)
      return n;
    if (*begin(token) == '"' && *(end(token) - 1) == '"')
      return unescape(token.substr(1, token.length() - 1));
    return symbol(token);
  }

  value parse_tokens(deque< string >& tokens)
  {
    check(!tokens.empty(), "attempting to parse an empty expression.");
    string token = tokens.front();
    tokens.pop_front();
    check(token != ")", "parentheses don't match.");
    if (token == "(") {
      list l;
      while (!tokens.empty() && tokens.front() != ")")
        l.push_back(parse_tokens(tokens));
      check(tokens.front() == ")", "parentheses don't match.");
      tokens.pop_front();
      return l;
    }
    return atom(token);
  }
  
  value parse(const string& code)
  {
    auto tokens = tokenize(code);
    return parse_tokens(tokens);
  }

  vector< string > split(const string& code)
  {
    vector< string > parts;
    int pos = 0;
    while (pos < code.length()) {
      string part;
      int paren_count = 0;
      bool empty = true;
      do {
        part += code[pos];
        if (code[pos] != ' ' && code[pos] != '\n' && code[pos] != '\t')
          empty = false;
        if (code[pos] == '(')
          ++paren_count;
        else if (code[pos] == ')')
          --paren_count;
        ++pos;
      } while (paren_count > 0);
      if (!empty)
        parts.push_back(part);
    }
    return parts;
  }

  bool paren_match(const string& code)
  {
    int paren_count = 0;
    for (char c: code)
      if (c == '(')
        ++paren_count;
      else if (c == ')') {
        if (paren_count == 0)
          return false;
        --paren_count;
      }
    return paren_count == 0;
  }
  
} // namespace lime
