#!/usr/bin/env bali

(setq x 3)

(write
  (append
    (if
      (> x 2)
      '(Hello)
      '(Goodbye))
    '(World!)))
