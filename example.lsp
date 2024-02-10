#!/usr/bin/env bali

(let x 3)

(write
  (append
    (if
      (> x 2)
      '(Hello)
      '(Goodbye))
    '(World!)))
