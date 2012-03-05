LIME: Lisp Implementation with Moderate Effort
==============================================
(was: Lisp Implementation with Minimal Effort)

Installation
------------

Dependencies (as in *only tested with*):

- gcc >= 4.7
- Boost >= 1.48
- bash 

Simply issue a `make install`. You may want to set up a symlink or alias in order to have the `lime` binary in your PATH.

Usage
-----

Either run a program with `lime path/to/myprogram.lm` or work interactively in the REPL by just running `lime`.
The REPL supports multi-line expressions as well as multiple expressions on a single line (can you believe it?).

Language overview
-----------------

Basic syntax:

- `(define x expr)` (define a symbol for the first time)
- `(set! x expr)` (redefine a symbol)
- `(begin expr1 expr2 ...)` (execute several expressions sequentially, in a nested environment; the last expression determines the return value of the whole block)
- `(if cond expr1 expr2)` (if `cond` evaluates to `true`, returns `expr1`, otherwise `expr2`)
- `(lambda (params) expr)` (create an anonymous function)

Supported variable types: int, string, bool, lambda, list, nil

Builtin functions:

- `nil`, `true`, `false` (trivial constructors)
- `list` (create a list with the arguments as elements)

    ```
    lime> (list 1 2 3)
    (1 2 3)
    lime> (define x 7)
    lime> (list 1 2 x)
    (1 2 7)
    lime> (list)
    ()
    ```

- `quote` (return the argument without evaluating it)

    ```
    lime> (quote ())
    ()
    lime> (quote (1 2 3))
    (1 2 3)
    lime> (define x 42)
    lime> (quote x)
    x
    ```

`(quote ())` and `(list)` are equivalent ways to build an empty list.

- `require` (evaluate the content of the file in the global environment; useful to load functions from external modules)

    ```
    lime> (require "path/to/myfile.lm")
    ```

- `=` (works with any builtin type, including lists)
- `<`, `+`, `-`, `*`, `/`, `%` (all binary operators for int)
- `random`, `rand-max` (`random` returns a pseudo-random integer between 0 and `rand-max` included)
- `and`, `or` (short-circuited logical operators)
- `atom?` (true if the argument is anything but a list)
- `empty?` (false if the argument is anything but the empty list)

    ```
    lime> (empty? empty)
    true
    lime> (empty? (list 1 2 3))
    false
    ```
  
- `cons` (construct a new list by appending an element on the front)

    ```
    lime> (cons 1 (list 2 3))
    (1 2 3)
    ```

- `head` (return the first element of a non-empty list)
- `tail` (return a list with all elements but the first one)
- `elem` (return a particular element of a list)

    ```
    lime> (elem 3 (list 6 2 5 2 8))
    5
    ```

- `print` (print the argument's value, without a newline)
- `read` (read an expression from standard input)

    ```
    lime> (define x (read))
    5
    lime> x
    5
    ```

User input is treated as code:

    lime> (define foo (read))
    (lambda (x) (+ x 1))
    lime> (foo 2)
    3

Partial function application is also a possibility. The following two definitions are equivalent:

    lime> (define succ1 (lambda (n) (+ 1 n)))
    lime> (define succ2 (+ 1))
    lime> (succ1 4)
    5
    lime> (succ2 4)
    5

This applies to functions of any number of arguments:

   lime> (define sum1 (fold +))
   lime> (sum1 0 (list 1 2 3))
   6
   lime> (define sum2 (fold + 0))
   lime> (sum2 (list 1 2 3))
   6

The language also provides lazy-evaluated, memoized streams as an alternative to lists:

- `cons-stream` (construct a stream from an element and a tail stream)

For example, this is how we build an infinite stream of ones:

    lime> (define ones (cons-stream 1 ones))
    lime> (elem-stream 3 ones)
    1
    lime> (elem-stream 45 ones)
    1

All the remaining functionality is provided in the standard library, as we shall see.

Library functions:

From `io.lm`:

- `println` (print the argument and append a newline)

- `print-stream`, `println-stream` (evaluate and print all elements of a non-infinite stream)

From `numeric.lm`:

- `neg` (negate the integer argument)
- `!=` (actually works for the same types as `=`)
- `>`, `>=`, `<=`
- `even`, `odd`
- `succ` (return the successor of an integer)
- `enum` (enumerate all integers starting from the argument; returns a stream)

    ```
    lime> (elem-stream 12 (enum 2))
    13
    ```

- `naturals` (the stream of all natural numbers)

    ```
    lime> (elem 42 naturals)
    42
    ```

- `sum`, `product` (sum/multiply the values in a list of integers)

    ```
    lime> (sum (list 1 2 3))
    6
    ```

- `sum-stream`, `product-stream` (for integer streams)

- `max`, `min` (for two integer arguments)
- `max-list`, `min-list` (for lists of integers)
- `max-stream`, `min-stream`

- `range`

    ```
    lime> (range 1 5)
    (1 2 3 4 5)
    ```

- `range-stream` (useful for large ranges)
- `square`, `pow`, `fact`

- `add` (add two integer lists together)

    ```
    lime> (add (list 4 5 1)
               (list 3 1 5))
    (7 6 6)
    ```

- `add-stream`

From `logic.lm`:

- `not`, `xor`

From `functional.lm`:

- `apply` (apply a function to an argument)

    ```
    lime> (zip-with apply (list even odd) (list 2 3))
    (true true)
    ```

- `compose` (compose two functions)

    ```
    lime> (define add1 (lambda (x) (+ x 1)))
    lime> (define multiply2 (lambda (x) (* x 2)))
    lime> ( (compose add1 multiply2) 3)
    7
    ```

- `flip` (exchange the arguments of a two-argument function)

It is particularly useful in the context of partial function application, when a two-argument operator is not commutative:

    lime> (define divide-by-2 ((flip /) 2))
    lime> (map divide-by-2 (list 2 4 6 8))
    (1 2 3 4)

From `list.lm`:

- `empty` (a short-hand for the empty list)
- `list?` (true if and only if the argument is a list)
- `len` (return the length of a list)
- `map`, `filter`, `fold` (usual higher-order functions)

    ```
    lime> (map (lambda (x) (* x 2)) (list 1 3 5))
    (2 6 10)
    lime> (filter odd (range 1 10))
    (1 3 5 7 9)
    lime> (fold * 1 (range 1 3))
    6
    ```

- `init` (return all the elements of a non-empty list but the last one)
- `last` (return the last element of a non-empty list)

    ```
    lime> (init (list 1 2 3))
    (1 2)
    lime> (last (list 1 2 3))
    3
    ```

- `for-each` (call a given procedure for each element of a list)

    ```
    lime> (for-each println (list "hey" "hello" "world"))
    hey
    hello
    world
    ```

- `take`, `drop` (take/drop the first n elements of a list)

    ```
    lime> (take 5 (range 1 10))
    (1 2 3 4 5)
    lime> (drop 5 (range 1 10))
    (6 7 8 9 10)
    ```

- `take-while`, `drop-while`

    ```
    lime> (take-while odd (list 7 9 11 4 6 7 5))
    (7 9 11)
    ```

- `zip`, `zip-with` (usual higher-order functions)

    ```
    lime> (zip (list 1 2 3) (list "a" "b" "c"))
    ((1 a) (2 b) (3 c))
    lime> (zip-with * (list 1 2 3) (list 3 2 1))
    (3 4 3)
    ```

- `count`, `count-if`

    ```
    lime> (count (list 1 2 3 2 4 2) 2)
    3
    lime> (count-if (range 1 10) even)
    5
    ```

- `all`, `any`

    ```
    lime> (all (map even (list 2 6 3 8)))
    false
    lime> (any (map even (list 2 6 3 8)))
    true
    ```

From `stream.lm`:

- `empty-stream`
- `head-stream`, `tail-stream`, `init-stream`, `last-stream`
- `elem-stream`
- `eq-stream` (test equality of non-infinite streams)
- `len-stream`
- `map-stream`, `filter-stream`, `fold-stream`
- `for-each-stream`
- `take-stream`, `drop-stream`, `take-while-stream`, `drop-while-stream`
- `zip-stream`, `zip-with-stream`
- `repeat` (repeat the argument infinite times)

    ```
    lime> (elem-stream 50 (repeat 7))
    7
    ```
- `enum-with` (like `enum`, but uses a custom successor function)

    ```
    lime> (define tf (enum-with not true))
    lime> (println-stream (take-stream 5 tf))
    (true false true false true)
    ```

Credits
-------

Peter Norvig's [Lispy](http://norvig.com/lispy.html) was a BIG source of inspiration.

Thanks to [ot](https://github.com/ot) for suggesting I use *boost::variant* to represent values.
