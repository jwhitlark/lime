#ifndef __INTERPRETER_HPP__
#define __INTERPRETER_HPP__

// STL headers
#include <memory>
#include <string>

// lime headers
#include <core.hpp>

namespace lime {
  // STL
  using std::string;

  // lime
  using lime::environment;

  const string prompt("lime> ");

  const string bye("\nBye.\n");

  const vector< string > stdlibs { "logic.lm",
                                   "imperative.lm",
                                   "functional.lm",
                                   "list.lm",
                                   "stream.lm",
                                   "numeric.lm", 
                                   "io.lm" };

  void check(bool test, const string& error_msg);

  void load_file(const string& path, shared_ptr< environment > env_p);

  void load_stdlib(shared_ptr< environment > env_p);

  void repl(shared_ptr< environment > env_p);

} // namespace lime

#endif // __INTERPRETER_HPP__
