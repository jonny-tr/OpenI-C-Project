.entry LIST, MAIN
.data 3, 34, -3, +5
.extern fn1, fn2
MAIN: add r3, LIST

  ; does it delete?

  sub, r3, #1
  sub , r3
  sub r3 #1
  sub r3, , r1
