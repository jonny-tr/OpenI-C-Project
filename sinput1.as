.entry LIST, MAIN
.data 3, 34, -3, +5
.extern fn1, fn2
MAIN: add r3, LIST

  ; does it delete?


 LIST : sub #1, r3
grow 23
.read #1
CHECK: LAB: dec r3, r2
