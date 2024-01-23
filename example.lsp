#!/usr/bin/env bali

(write
  (append
    (if
      (> 10 2)
      '(Hello)
      '(Goodbye))
    '(World!)))
