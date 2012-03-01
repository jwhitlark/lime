// STL headers
#include <memory>

// lime headers
#include <builtins.hpp>
#include <interpreter.hpp>

// STL
using std::make_shared;
using std::shared_ptr;

// lime
using lime::add_builtins;
using lime::environment;
using lime::load_file;
using lime::load_stdlib;
using lime::repl;

int main(int argc, char *argv[])
{
  auto env_p = make_shared< environment >();
  add_builtins(env_p);
  load_stdlib(env_p);
  if (argc == 1)
    repl(env_p);
  else
    load_file(argv[1], env_p);
}
