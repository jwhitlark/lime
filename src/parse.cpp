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

  string preprocess(const string& code)
  {
    string new_code;
    bool string_expr = false;
    for (char c: code)
      if (c == '(' && !string_expr)
        new_code += " ( ";
      else if (c == ')' && !string_expr)
        new_code += " ) ";
      else if (c == ' ' && string_expr)
        new_code += "\\s";
      else if (c == '\n' && string_expr)
        new_code += "\\n";
      else {
        if (c == '"')
          string_expr = !string_expr;
        new_code.push_back(c);
      }
    return new_code;
  }

  deque< string > tokenize(const string& code)
  {
    istringstream iss(preprocess(code));
    deque< string > tokens;
    copy(istream_iterator< string >(iss),
         istream_iterator< string >(),
         back_inserter(tokens));
    return tokens;
  }

  string escape(const string& str)
  {
    string escaped;
    for (char c: str)
      if (c == '\n')
        escaped += "\\n";
      else
        escaped.push_back(c);
    return escaped;
  }

  string unescape(const string& str)
  {
    string unescaped;
    bool unescaped_last = false;
    for (int i = 0; i + 1 < str.length(); ++i)
      if (str[i] == '\\' && str[i + 1] == 'n') {
        unescaped.push_back('\n');
        ++i;
        unescaped_last = true;
      }
      else if (str[i] == '\\' && str[i + 1] == 's') {
        unescaped.push_back(' ');
        ++i;
        unescaped_last = true;
      }
      else {
        unescaped.push_back(str[i]);
        unescaped_last = false;
      }
    if (!unescaped_last)
      unescaped.push_back(str.back());
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
      return unescape(token.substr(1, token.length() - 2));
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

  bool is_separator(char c)
  {
    return c == ' ' || c == '\n' || c == '\t';
  }

  vector< string > split(const string& code)
  {
    vector< string > parts;
    int pos = 0;
    while (pos < code.length()) {
      string part;
      int paren_count = 0;
      bool empty = true;
      bool list_expr = false;
      bool string_expr = false;
      do {
        part += code[pos];
        if (code[pos] != ' ' && code[pos] != '\n' && code[pos] != '\t')
          empty = false;
        if (code[pos] == '"')
          string_expr = !string_expr;
        if (code[pos] == '(' && !string_expr) {
          ++paren_count;
          list_expr = true;
        }
        else if (code[pos] == ')' && !string_expr)
          --paren_count;
        ++pos;
      } while (paren_count > 0 || (!list_expr && pos < code.length() &&
                                   !is_separator(code[pos])));
      if (!empty)
        parts.push_back(part);
    }
    return parts;
  }

  bool paren_match(const string& code)
  {
    int paren_count = 0;
    bool string_expr = false;
    for (char c: code)
      if (c == '"')
        string_expr = !string_expr;
      else if (c == '(' && !string_expr)
        ++paren_count;
      else if (c == ')' && !string_expr) {
        if (paren_count == 0)
          return false;
        --paren_count;
      }
    return paren_count == 0;
  }
  
} // namespace lime
