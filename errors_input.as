testing1: .entry ent1, ent2
testing2: .extern ext1, ext2
 
 ; space before colon
 LIST : sub #1, r3

 ; undefined command
grow 23, 34
.read #1

; 2 labels
CHECK: LAB: dec r3, r2

; not enough operands
 cmp #5
 jmp

; too many operands
  stop f, *r1
  jmp r7, r3

; wrong addressing method
add *r5, #15
not #0

; wrong comma use
.data , 3, 34, -3, +5
.data 3, , 34, -3, +5
.data 3, , 34, -3, +5 ,

; command word as label
prn: prn r2

; no code after the label
START:

; existing label
Lab: add *r5, f
Lab: lea r, *r1

; invalid operands
.data +one
clr #three
clr add, 1
clr *r10, 
add #+e, r2

