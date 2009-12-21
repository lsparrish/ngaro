#!/usr/bin/wish

# Ngaro for tcl/tk
# Copyright 2009 Luke Parrish
# License: CC0

set stack  0; # stack
set buffer 0; # data buffer
set sp 0;     # stack pointer
set op 0;     # opcode

proc opeval { $op } {
    switch opcode {
        0 nop    1 lit    2 dup    3 drop    
        4 swap    5 push    6 pop    7 call
        8 jump    9 return    10 gt_jump    11 lt_jump
        12 ne_jump    13 eq_jump    14 fetch    15 store
        16 add    17 subtract    18 multiply    19 divmod
        20 and    21 or    22 xor    23 shift_left
        24 shift_right    25 zero_return    26 inc    27 dec
        28 in    29 out    30 wait
    }
}

proc nop { } {
    
}
proc lit { } {
    
}
proc dup { } {
    
}
proc drop { } {
    
}
proc swap { } {
    
}
proc push { } {
    
}
proc pop { } {
    
}
proc call { } {
    
}
proc jump { } {
    
}
proc return { } {
    
}
proc gt_jump { } {
    
}
proc lt_jump { } {
    
}
proc ne_jump { } {
    
}
proc eq_jump { } {
    
}
proc fetch { } {
    
}
proc store { } {
    
}
proc add { } {
    
}
proc subtract { } {
    
}
proc multiply { } {
    
}
proc divmod { } {
    
}
proc and { } {
    
}
proc or { } {
    
}
proc xor { } {
    
}
proc shift_left { } {
    
}
proc shift_right { } {
    
}
proc zero_return { } {
    
}
proc inc { } {
    
}
proc dec { } {
    
}
proc in { } {
    
}
proc out { } {
    
}
proc wait { } {
    
}

proc vm { op  } {
    
}
