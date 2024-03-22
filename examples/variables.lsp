#!/usr/bin/env bali

; Define an variable.
(setq value 0)

; Define function that increments value of the variable defined in outer
; variable scope.
(defun incr () (setq value (+ value 1)))

; Call the incrementation function few times...
(incr) (incr) (incr)

; And output the resulting value.
(write value)
