#!/usr/bin/env bali

; Define named function that does something.
(defun adder (x y) (+ x y))

; And then call it.
(write (adder 5 10))

; Create anonymous function and call it.
(write (apply (lambda (x y) (* x y)) '(2 10)))
