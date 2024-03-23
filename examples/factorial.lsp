#!/usr/bin/env bali

(defun factorial (x)
  (if (= 0 x)
    1
    (* x (factorial (- x 1)))))

(write (factorial 5))
