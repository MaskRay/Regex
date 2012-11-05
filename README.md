# Regex

A parody of http://code.google.com/p/re1/ with the intention to provide a
simple, concise and constructive regex engine implementation.

The parser part is a proof-of-concept design employing Shunting-yard algorithm
to illustrate to feasibility to process unary operators and surrounds like
parentheses.

# Syntax

    a?    match 1 or 0 times
    a*    match 0 or more times
    a+    match 1 or more times
    a??   match 1 or 0 times, not greedily
    a*?   match 0 or more, not greedily
    a+?   match 1 or more, not greedily
    .     match any character
    |     alternative
    (a)   capturing parentheses

# Build

    make

# Usage

    ./main '(ray|yar)' 'hello, ray'
