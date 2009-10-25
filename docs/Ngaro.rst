========
Overview
========

Ngaro is a portable virtual machine and emulator for a
stack computer. It is designed to be small, easy to
implement, and easy to extend for specific needs.


===============
Instruction Set
===============

The instruction set is small, consisting of 31 primary
instructions. (Some implementations have extended instruction
sets).

Encoding is kept minimal. One instruction per memory location,
with certain instructions taking a value from the following
location.

All opcodes in the list below are in decimal.

======   ===========   =============================================================
Opcode   Name          Description
000      nop           Do nothing
001      lit           Push a value to the stack. The value is taken from the following memory location.
002      dup           Duplicate TOS
003      drop          Lose TOS
004      swap          Swap TOS and NOS
005      push          Move a value from data stack to the
                       address stack
006      pop           Move a value from address stack to data
                       stack
007      call          Call a subroutine. The address of the subroutine is in the following memory location.
008      jump          Jump to a new address. The address is in the following memory location.
009      return        Return from a call
010      gt_jump       Jump if NOS is greather than TOS. The address is in the following memory location.
011      lt_jump       Jump if NOS is less than TOS. The address is in the following memory location.
012      ne_jump       Jump if TOS and NOS are not equal. The address is in the following memory location.
013      eq_jump       Jump if TOS and NOS are equal. The address is in the following memory location.
014      fetch         Fetch from address at TOS
015      store         Store NOS to address in TOS
016      add           Add TOS to NOS
017      subtract      Subtract TOS from NOS
018      multiply      Multiply TOS and NOS
019      divmod        Divide and get Remainder
020      and           Bitwise AND
021      or            Bitwise OR
022      xor           Bitwise XOR
023      shift_left    Shift bits left
024      shift_right   Shift bits right
025      zero_return   Return and drop TOS if TOS is 0. If TOS is not 0, do nothing.
026      inc           Increment TOS by 1
027      dec           Decrement TOS by 1
028      in            Read a value from an I/O port
029      out           Send a value to an I/O port
030      wait          Wait for an I/O event
======   ===========   =============================================================


===========
I/O Devices
===========

Ngaro provides a minimal set of hardware devices, accessible via a collection of I/O
ports. The *in*, *out*, and *wait* instructions are used to access these.


====  ===============================
Port  Description
000   Device wait state
001   Keyboard Input
002   Console Output
003   Force Console Update
004   Save Image
005   Capabilities Query
006   Canvas Output
007   Mouse Input
====  ===============================

Port 0: Trigger I/O Event
-------------------------
To tell Ngaro that an I/O event is coming, write the appropiate values to one of the
ports, write *0* to port *0*, then invoke the *wait* instruction.

| lit 0  # value
| lit 0  # port
| out    # write value to port
| wait   # wait for an I/O event


Port 1: Keyboard Input
----------------------
Reading from the keyboard is easy. Send *1* to port *1* and then trigger an I/O event.
After the I/O event returns, read the keypress from port *1*. If a non-keyboard event
is received, the read result will be *0*, otherwise it will be the ASCII value of the
pressed key.

| lit 1
| lit 1  # keyboard port
| out
| # --- wait for I/O event ---
| lit 0
| lit 0
| out
| wait
| # --- read keypress ---
| lit 1
| in


Port 2: Console Output
----------------------
Writing a character to the console is also easy. Push the ASCII code to the
screen, then send *1* to port *2* and trigger an I/O event.

| lit 98 # ASCII code for 'b'
| lit 1
| lit 2  # console output port
| out
| # --- wait for I/O event ---
| lit 0
| lit 0
| out
| wait


Port 3: Force Console Update
----------------------------
The Ngaro VM is allowed to cache updates to the console (and canvas). You can
force a screen update using port *3*.

Port *3* is normally set to *1*. To trigger a screen update, send *0* to it.

| lit 0
| lit 3  # force screen update
| out

*Note: You do _not_ need to wait for an I/O event on this port.*


Port 4: Save the Image
----------------------
Port 4 is used to save the image. Send *1* to port *4* and trigger an I/O event.

*Note: Not all implementations allow this to be done.*

Port 5: Query Capabilities
--------------------------
Port 5 is used to query Ngaro about the provided hardware emulation and
some aspects of the processor state.

Send one of the following to port *5*, trigger an I/O event, then read
port *5* to get the results.

=====   ======================
Value   Action
-1      Return amount of memory
-2      Is canvas present? -1 if true, 0 if false
-3      Get canvas width
-4      Get canvas height
-5      Get stack depth
-6      Get address stack depth
-7      Is mouse present? -1 if true, 0 if false
=====   ======================

Port 6: Draw on the Canvas
--------------------------
Port 6 is used to draw on the canvas.

=====   ======================
Value   Action
001     Set color. Takes *color* value from the stack.
002     Draw a pixel. Takes *x* and *y* from the stack.
003     Draw a hollow rectangle. Takes *width*, *height*, *x*, and *y* from the stack.
004     Draw a filled rectangle. Takes *width*, *height*, *x*, and *y* from the stack.
005     Draw a vertical line. Takes *height*, *x*, and *y* from the stack.
006     Draw a horizontal line. Takes *width*, *x*, and *y* from the stack.
007     Draw a hollow circle. Takes *width*, *x*, and *y* from the stack.
008     Draw a filled circle. Takes *width*, *x*, and *y* from the stack.
=====   ======================

*Note: Not all implementations will support the canvas.*

Port 7: Interact with the Mouse
-------------------------------
Port 7 is used to interact with the mouse device.

To obtain mouse position, send *1* to port *7* and trigger an I/O
event.

| lit 1
| lit 7
| out
| # --- wait for I/O event ---
| lit 0
| lit 0
| out
| wait

The mouse coordinates will be placed on the stack. The Y coordinate
will be TOS, and the X coordinate will be NOS.

To obtain the button press state, send *2* to port *7* and trigger
an I/O event.

| lit 2
| lit 7
| out
| # --- wait for I/O event ---
| lit 0
| lit 0
| out
| wait

The button state will be placed on the stack. *1* if a button is
pressed, or *2* if a button is not pressed.
