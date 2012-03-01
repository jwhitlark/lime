LIME: Lisp Implementation with Moderate Effort
==============================================
(was: Lisp Implementation with Minimal Effort)

Installation
------------

Dependencies (as in *only tested with*):
- gcc >= 4.7
- Boost >= 1.48
- bash 

Simply issue a **make install**. You may want to set up a symlink or alias in order to have the **lime** binary in your PATH.

Usage
-----

Either run a program with **lime path/to/myprogram.lm** or work interactively in the REPL by just running **lime**.
The REPL supports multi-line expressions as well as multiple expressions on a single line (can you believe it?).

Language overview
-----------------

Basic syntax:
- **(define x expr)** (define a symbol for the first time)
- **(set! x expr)** (redefine a symbol)
- **(begin expr1 expr2 ...)** (execute several expressions sequentially, in a nested environment; the last expression determines the return value of the whole block)
- **(quote x)** (return x without evaluating it)

    lime> (quote ())
    ()
    lime> (quote (1 2 3))
    (1 2 3)

**(quote ())** is the empty list.

- **(if cond expr1 expr2)** (if **cond** evaluates to **true**, returns **expr1**, otherwise **expr2**)
- **(lambda (params) expr)** (create an anonymous function)
- **(require "path/to/myfile.lm")** (evaluate the content of the file in the global environment; useful to load functions from external modules)

Supported variable types: int, string, bool, lambda, list, nil

Builtin functions:
- **nil**, **true**, **false** (trivial constructors)
- **=** (works with int, string, bool)
- **<**, **+**, **-**, **\***, **/**, **%** (all binary operators for int)
- **null?** (whether a list is empty or not)
    
    lime> (null? (quote ()))
    true
    lime> (null? (quote (1 2 3)))
    false
  
- **cons** (construct a new list by appending an element on the front)

    lime> (cons 1 (quote (2 3)))
    (1 2 3)

- **head** (return the first element of a non-empty list)
- **tail** (return a list with all elements but the first one)
- **print** (print the argument's value, without a newline)
- **read** (read an expression from standard input)

    lime> (define x (read))
    5
    lime> x
    5

User input is treated as code:

    lime> (define foo (read))
    (lambda (x) (+ x 1))
    lime> (foo 2)
    3

Library functions:
From **io.lm**:
- **println** (print the argument and append a newline)

From **numeric.lm**:
- **neg** (negate the integer argument)
- **!=** (actually works for the same types as **=**)
- **>**, **>=**, **<=**
- **even**, **odd**
- **sum**, **product** (sum/multiply the values in a list of integers)

    lime> (sum (quote (1 2 3)))
    6

- **range**

    lime> (range (1 5))
    (1 2 3 4 5)

- **square**, **pow**, **fact**

From **functional.lm**:
- **not** (negate a boolean value)
- **compose** (compose two functions)

    lime> (define add1 (lambda (x) (+ x 1)))
    lime> (define multiply2 (lambda (x) (* x 2)))
    lime> ( (compose add1 multiply2) 3)
    7

From **list.lm**:
- **len** (return the length of a list)
- **map**, **filter**, **fold** (usual higher-order functions)

    lime> (map (lambda (x) (* x 2)) (quote (1 3 5)))
    (2 6 10)
    lime> (filter odd (range (1 10)))
    (1 3 5 7 9)
    lime> (fold * (range 1 3))
    6

Credits
-------

Peter Norvig's [Lispy](http://norvig.com/lispy.html) was a BIG source of inspiration.

Thanks to [ot](https://github.com/ot) for suggesting I use *boost::variant* to represent values.
