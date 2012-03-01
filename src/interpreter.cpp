// C headers
#include <cstdlib>

// STL headers
#include <fstream>
#include <iostream>
#include <streambuf>

// lime headers
#include <core.hpp>
#include <eval.hpp>
#include <interpreter.hpp>
#include <parse.hpp>

namespace lime {
  // STL
  using std::cin;
  using std::cout;
  using std::endl;
  using std::getline;
  using std::ifstream;
  using std::istreambuf_iterator;

  // Boost
  using boost::apply_visitor;
  using boost::static_visitor;

  // lime
  using lime::eval;
  using lime::paren_match;
  using lime::parse;
  using lime::print_stream;
  using lime::split;

  void check(bool test, const string& error_msg)
  {
    if (!test) {
      cout << "ERROR: " << error_msg << endl;
      exit(1);
    }
  }

  void load_file(const string& path, shared_ptr< environment > env_p)
  {
    ifstream source_file(path);
    check(source_file, "could not open source file '" + path + "'.");
    string code((istreambuf_iterator< char >(source_file)),
                istreambuf_iterator< char >());
    source_file.close();
    vector< string > parts = split(code);
    for (string part: parts)
      eval(parse(part), env_p);
  }

  void load_stdlib(shared_ptr< environment > env_p)
  {
    string interpreter_path = getenv("_");
    string bin_path = interpreter_path.substr(0, interpreter_path.length() - 4);
    string lib_path = bin_path + "../lib/";
    for (string filename: stdlibs)
      load_file(lib_path + filename, env_p);
  }

  class return_value_visitor : public static_visitor<> {
  public:
    void operator()(const nil& n) const {}
    template< typename T >
    void operator()(const T& t) const
    {
      print_stream(cout, t);
      cout << endl;
    }
  };

  void repl(shared_ptr< environment > env_p)
  {
    cout << prompt;
    string line;
    while (getline(cin, line)) {
      string code = line;
      while (!paren_match(code)) {
        if (!getline(cin, line)) {
          cout << bye;
          return;
        }
        code += " " + line;
      }
      vector< string > parts = split(code);
      for (auto it = begin(parts); it + 1 < end(parts); ++it)
        eval(parse(*it), env_p);
      if (!parts.empty()) {
        value retval = eval(parse(parts.back()), env_p);
        apply_visitor(return_value_visitor(), retval);
      }
      cout << prompt;
    }
    cout << bye;
  }
  
} // namespace lime
