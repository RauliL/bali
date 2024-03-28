#!/usr/bin/env bali -m

fib[n] <= if[
  or[n = 0; n = 1];
  n;
  fib[n - 1] + fib[n - 2]
]

write[fib[20]]
