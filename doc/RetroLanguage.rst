The Retro Language
==================


===========================
Section 1: Welcome to Retro
===========================

Retro is a small implementation of the Forth language. Running on
a portable virtual machine, it is minimalistic in nature, yet has
a number of interesting and useful features.

While this implementation is new, there is a lot of history behind
the Retro name. It was originally developed in 1998 as a 16-bit
bootable Forth, evolved into a 32-bit protected mode Forth that
served as a prototype testbed for the Tunes project, and eventually
picked up some ideas from colorForth before being rewritten to run
on traditional operating systems. Along the way it has influenced
the development of many other Forth systems.

==========================
Section 2: Getting Started
==========================

Obtaining
---------
Retro is generally distributed as source code and needs to be compiled
before it can be used. The latest development snapshots and periodic
stable release snapshots are provided at http://retroforth.org

Building (Unix-like host)
-------------------------
These instructions are for users of Linux, BSD, OS X, BeOS, AIX, etc.
Windows users should see the next section.

You'll need a few things:

- GCC (4.x recommended)
- Binutils
- Make
- Curses (curses, ncurses, and pdcurses all work fine) and/or libSDL

Given these, try the following command:

| make vm

Or, if you want the framebuffer backend to be used:

| make fbvm

If you get any errors, check the **vm/console/retro.c** and
**vm/console/retro-fast.c** and see if the **LIBS** line matches
the Curses varient used on your system.

Building (Windows)
------------------
- Install Dev C++ (http://prdownloads.sourceforge.net/dev-cpp/devcpp-4.9.9.2 setup.exe)
- Install the Curses Devpack (http://devpaks.org/details.php?devpak=5)
- Create an empty project and add the following files from **vm/console** to it:

  - functions.h
  - disassemble.c
  - endian.c
  - loader.c
  - ngaro.c
  - devices.c
  - initial_image.c
  - vm.c
  - vm.h

- Go to Project -> Options and add the following to the Linker commands: **-lcurses**
- Compile everything

Running
-------

===================
Section 3: Concepts
===================

Word Classes
------------
Retro’s interpreter makes use of an implementation technique
known as word classes. This approach was created by Helmar
Wodtke and allows for a very clean interpreter and compiler. It
makes use of special words, called class handlers, to process
execution tokens. Each word in the dictionary has a class
handler associated with it. When being executed, the address of
the wordis pushed to the stack and the class handler is invoked.
The handler then does something with the address based on
various bits of state.

The standard Retro language has five classes defined.

+-----------+------------+-----------------------------------------+
| Name      | Data Stack | Address Stack                           |
+===========+============+=========================================+
| .forth    | a -        | ``-``                                   |
+-----------+------------+-----------------------------------------+
| If interpreting, call the word. If compiling, compile a call     |
| to the word.                                                     |
+-----------+------------+-----------------------------------------+
| .macro    | a -        | ``-``                                   |
+-----------+------------+-----------------------------------------+
| Always call the word. This is normally used for words that lay   |
| down custom code at compile time, or which need to have          |
| different behaviors during compilation.                          |
+-----------+------------+-----------------------------------------+
| .inline   | a -        | ``-``                                   |
+-----------+------------+-----------------------------------------+
| If interpreting, call the word. If compiling, copy the first     |
| opcode of the word into the target definition. This is only      |
| useful for use with words that map directly to processor opcodes.|
+-----------+------------+-----------------------------------------+
| .data     | a -        | ``-``                                   |
+-----------+------------+-----------------------------------------+
| If interpreting, leave the address on the stack. If compiling,   |
| compile the address into the target definition as a literal.     |
+-----------+------------+-----------------------------------------+
| .compiler | a - a      | ``-``                                   |
+-----------+------------+-----------------------------------------+
| If compiling, execute the word. If interpreting, ignore the      |
| word.                                                            |
+-----------+------------+-----------------------------------------+

It is possible to define custom classes. The easiest way to
show how to add a new class is with an example. For this, we'll
create a class for strings with the following behavior:

  - If interpreting, display the string
  - If compiling, lay down the code needed to display the
    string

Retro has a convention of using a . as the first character of a
class name. In continuing this tradition, we'll call our new
class .string

Tip:
  On entry to a class, the address of the word or data
  structure is on the stack. The compiler state (which most
  classes will need to check) is in a variable named compiler.

A first step is to lay down a simple skeleton. Since we need to
lay down custom code at compile time, the class handler will
have two parts.

|  : .string  ( a—)
|    compiler @ 0 =if ( interpret time ) ;; then ( compile time )
|  ;

We'll start with the interpret time action. We can replace this
with type, since the whole point of this class is to display a
string object.

|  : .string ( a — )
|    compiler @ 0 =if type ;; then ( compile time ) ;

The compile time action is more complex. We need to lay down
the machine code to leave the address of the string on the
stack when the word is run, and then compile a call to type. If
you look at the instruction set listing, you'll see that opcode
1 is the instruction for putting values on the stack. This
opcode takes a value from the following memory location and
puts it on the stack. So the first part of the compile time
action is:

|  : .string ( a — )
|    compiler @ 0 =if type ;; then 1 , , ;

Tip:
  Use , to place values directly into memory. This is the
  cornerstone of the entire compiler.

One more thing remains. We still have to compile a call to
type. We can do this by passing the address of type to
compile.

|  : .string ( a — )
|    compiler @ 0 =if type ;; then 1 , , ['] type compile ;

And now we have a new class handler. The second part is to make
this useful. We'll make a creator word called displayString: to
take a string and make it into a new word using our .string
class. This will take a string from the stack, make it
permanent, and give it a name.

Tip:
  New dictionary entries are made using create. The class can
  be set after creation by accessing the proper fields in the
  dictionary header. Words starting with d-> are used to access
  fields in the dictionary headers.

|  : displayString: ( "name" — )
|    create ['] .string last @ d- >class ! keepString last @ d->xt ! ;

This uses create to make a new word, then sets the class to
.string and the xt of the word to the string. It also makes the
string permanent using keepString. last is a variable pointing
to the most recently created dictionary entry. The two words
d->class and d->xt are dictionary field accessors and are used
to provide portable access to fields in the dictionary.

We can now test the new class:

|  " hello, world!" displayString: hello
|  hello
|  : foo hello cr foo ;

You can use this approach to define as many classesas you want.




Interpreter
-----------
Retro has a simple interpreter. The interpreter calls accept,
passing it the ascii value 32 (for space) as a delimiter. Input
is accepted and added to the tib until the delimiter is
encountered. At this point, the interpreter cycles through the
dictionary, comparing the input in tib to the name of each
entry. This loop goes from the newest to the oldest entry, and
exits when a match is found. If a match was found, the xt of
the word is pushed to the stack and the class handler attached
to its dictionary entry is called. This handler is responsible
for handling the xt and carrying out the proper behavior for
the word.

If a match is not found, the interpreter tries to convert the
token to an integer. If successful, the integer value is pushed
to the stack and the .data class handler is called.

If a conversion to integer failed, and no match was found, the
interpreter calls notfound to report the error.

This process is then repeated until Retro is shutdown.

Tip:
  Retro has no separate compiler loop. Instead, each class
  handler is responsible for compiling the code for words
  associated with it. This allows a very straightforward
  interpreter loop.




Threading
---------
Retro uses subroutine threading with inline machine code for
some words. This was chosen primarily due to its simplicity,
but also for historical reasons. (All Retro implementations
since 2001 have been primarily subroutine threaded).

The subroutine threading model compiles code to native machine
code, primarily as a series of calls to other routines.

As an example:

|  : foo 1 2 + . ;

This will compile to:

|  lit 1
|  lit 2
|  call +
|  call .
|  ;

The subroutine threading model allows a lot of opportunity for
optimization. Recent releases of Retro support inline machine
code generation for primitives, so the above example can now
compile to:

|  lit 1
|  lit 2
|  +
|  call .
|  ;


Vectors
-------
Vectors are another important concept in Retro.

Most Forth systems provide a way to define a word which can
have its meaning altered later. Retro goes a step further by
allowing all words defined using : or macro: to be redefined.
Words which can be redefined are called vectors.

Vectors can be replaced by using is, or returned to their
original definition with devector. For instance:

|  : foo 23 . ;
|  foo
|  : bar 99 . ;
|  ' bar is foo
|  foo
|  devector foo
|  foo

There are also variations of is and devector which take the
addresses of the words rather than parsing for the word name.
These are :is and :devector.




====================
Section 4: The Words
====================

Reading Stack Comments
----------------------
Stack comments in Retro are a compact form, using short codes
in place of actual words. These codes are listed in the next
section.

A typical comment for a word that takes two arguments and
leaves one will look like:

|  ( xy-z )

In a few cases, words may consume or leave a variable number
of arguments. In this case, we denote it like:

|  ( n-n || n- )

There are two other modifiers in use. Some words have different
compile-time and run-time stack use. We prefix the comment with
C: for compile-time, and R: for run-time actions.

If not specified, the stack comments are for runtime effects.
Words with no C: are assumed to have no stack impact during
compilation.

Codes used in the stack comments:

+------------+------------------------------------+
| x, y, z, n | Generic numbers                    |
+------------+------------------------------------+
| q, r       | Quotient, Remainder (for division) |
+------------+------------------------------------+
| ``"`` *    | Word parses for a string           |
+------------+------------------------------------+
| a          | Address                            |
+------------+------------------------------------+
| c          | ASCII character                    |
+------------+------------------------------------+
| ``$``      | Zero-terminated string             |
+------------+------------------------------------+
| f          | Flag                               |
+------------+------------------------------------+
| ...        | Variable number of values on stack |
+------------+------------------------------------+


List of Words by Class
----------------------

+--------------+--------------+------------+---------------+
| Name         | Class        | Data Stack | Address Stack |
+==============+==============+============+===============+
| 1+           | .inline      | x-y        |               |
+--------------+--------------+------------+---------------+
| Increment x by 1                                         |
+--------------+--------------+------------+---------------+
| 1-           | .inline      | x-y        |               |
+--------------+--------------+------------+---------------+
| Decrement x by 1                                         |
+--------------+--------------+------------+---------------+
| swap         | .inline      | xy-yx      |               |
+--------------+--------------+------------+---------------+
| Exchange the positions of the top two stack items        |
+--------------+--------------+------------+---------------+
| drop         | .inline      | xy-x       |               |
+--------------+--------------+------------+---------------+
| Remove the top item from the stack                       |
+--------------+--------------+------------+---------------+
| and          | .inline      | xy-z       |               |
+--------------+--------------+------------+---------------+
| Bitwise AND                                              |
+--------------+--------------+------------+---------------+
| or           | .inline      | xy-z       |               |
+--------------+--------------+------------+---------------+
| Bitwise OR                                               |
+--------------+--------------+------------+---------------+
| xor          | .inline      | xy-z       |               |
+--------------+--------------+------------+---------------+
| Bitwise XOR                                              |
+--------------+--------------+------------+---------------+
| @            | .inline      | a-n        |               |
+--------------+--------------+------------+---------------+
| Fetch a value from an address                            |
+--------------+--------------+------------+---------------+
| !            | .inline      | na-        |               |
+--------------+--------------+------------+---------------+
| Store value n into address a                             |
+--------------+--------------+------------+---------------+
| ``+``        | .inline      | xy-z       |               |
+--------------+--------------+------------+---------------+
| Add x to y                                               |
+--------------+--------------+------------+---------------+
| ``-``        | .inline      | xy-z       |               |
+--------------+--------------+------------+---------------+
| Subtract y from x                                        |
+--------------+--------------+------------+---------------+
| ``*``        | .inline      | xy-z       |               |
+--------------+--------------+------------+---------------+
| Multiply x and y                                         |
+--------------+--------------+------------+---------------+
| /mod         | .inline      | xy-qr      |               |
+--------------+--------------+------------+---------------+
| Divide x and y, getting the quotient and remainder       |
+--------------+--------------+------------+---------------+
| <<           | .inline      | xy-z       |               |
+--------------+--------------+------------+---------------+
| Shift x left by y bits                                   |
+--------------+--------------+------------+---------------+
| >>           | .inline      | xy-z       |               |
+--------------+--------------+------------+---------------+
| Shift x right by y bits                                  |
+--------------+--------------+------------+---------------+
| nip          | .inline      | xy-y       |               |
+--------------+--------------+------------+---------------+
| Drop the second item on the stack                        |
+--------------+--------------+------------+---------------+
| dup          | .inline      | x-xx       |               |
+--------------+--------------+------------+---------------+
| Duplicate the top stack item                             |
+--------------+--------------+------------+---------------+
| in           | .inline      | x-y        |               |
+--------------+--------------+------------+---------------+
| Read a value from an I/O port                            |
+--------------+--------------+------------+---------------+
| out          | .inline      | xy-        |               |
+--------------+--------------+------------+---------------+
| Send a value to an I/O port                              |
+--------------+--------------+------------+---------------+
| here         | .word        | -a         |               |
+--------------+--------------+------------+---------------+
|                                                          |
+--------------+--------------+------------+---------------+
| ,            | .word        | n-         |               |
+--------------+--------------+------------+---------------+
|                                                          |
+--------------+--------------+------------+---------------+
| ]            | .word        |            |               |
+--------------+--------------+------------+---------------+
|                                                          |
+--------------+--------------+------------+---------------+
| create       | .word        | "-         |               |
+--------------+--------------+------------+---------------+
|                                                          |
+--------------+--------------+------------+---------------+
| :            | .word        | "-         |               |
+--------------+--------------+------------+---------------+
|                                                          |
+--------------+--------------+------------+---------------+
| macro:       | .word        | "-         |               |
+--------------+--------------+------------+---------------+
|                                                          |
+--------------+--------------+------------+---------------+
| compiler:    | .word        | "-         |               |
+--------------+--------------+------------+---------------+
|                                                          |
+--------------+--------------+------------+---------------+
| accept       | .word        | n-         |               |
+--------------+--------------+------------+---------------+
|                                                          |
+--------------+--------------+------------+---------------+
| cr           | .word        |            |               |
+--------------+--------------+------------+---------------+
|                                                          |
+--------------+--------------+------------+---------------+
| emit         | .word        | c-         |               |
+--------------+--------------+------------+---------------+
|                                                          |
+--------------+--------------+------------+---------------+
| type         | .word        | $-         |               |
+--------------+--------------+------------+---------------+
|                                                          |
+--------------+--------------+------------+---------------+
| clear        | .word        |            |               |
+--------------+--------------+------------+---------------+
|                                                          |
+--------------+--------------+------------+---------------+
| words        | .word        |            |               |
+--------------+--------------+------------+---------------+
|                                                          |
+--------------+--------------+------------+---------------+
| key          | .word        | -c         |               |
+--------------+--------------+------------+---------------+
|                                                          |
+--------------+--------------+------------+---------------+
| over         | .word        | xy-xyx     |               |
+--------------+--------------+------------+---------------+
|                                                          |
+--------------+--------------+------------+---------------+
| 2drop        | .word        | xy-        |               |
+--------------+--------------+------------+---------------+
|                                                          |
+--------------+--------------+------------+---------------+
| not          | .word        | x-y        |               |
+--------------+--------------+------------+---------------+
|                                                          |
+--------------+--------------+------------+---------------+
| rot          | .word        | xyz-yzx    |               |
+--------------+--------------+------------+---------------+
|                                                          |
+--------------+--------------+------------+---------------+
| -rot         | .word        | xyz-xzy    |               |
+--------------+--------------+------------+---------------+
|                                                          |
+--------------+--------------+------------+---------------+
| tuck         | .word        | xy-yxy     |               |
+--------------+--------------+------------+---------------+
|                                                          |
+--------------+--------------+------------+---------------+
| 2dup         | .word        | xy-xyxy    |               |
+--------------+--------------+------------+---------------+
|                                                          |
+--------------+--------------+------------+---------------+
| on           | .word        | a-         |               |
+--------------+--------------+------------+---------------+
|                                                          |
+--------------+--------------+------------+---------------+
| off          | .word        | a-         |               |
+--------------+--------------+------------+---------------+
|                                                          |
+--------------+--------------+------------+---------------+
| /            | .word        | xy-q       |               |
+--------------+--------------+------------+---------------+
|                                                          |
+--------------+--------------+------------+---------------+
| mod          | .word        | xy-r       |               |
+--------------+--------------+------------+---------------+
|                                                          |
+--------------+--------------+------------+---------------+
| neg          | .word        | x-y        |               |
+--------------+--------------+------------+---------------+
|                                                          |
+--------------+--------------+------------+---------------+
| execute      | .word        | a-         |               |
+--------------+--------------+------------+---------------+
|                                                          |
+--------------+--------------+------------+---------------+
| "            | .word        | "-$        |               |
+--------------+--------------+------------+---------------+
|                                                          |
+--------------+--------------+------------+---------------+
| compare      | .word        | $$-        |               |
+--------------+--------------+------------+---------------+
|                                                          |
+--------------+--------------+------------+---------------+
| wait         | .word        |            |               |
+--------------+--------------+------------+---------------+
|                                                          |
+--------------+--------------+------------+---------------+
| '            | .word        | "-a        |               |
+--------------+--------------+------------+---------------+
|                                                          |
+--------------+--------------+------------+---------------+
| @+           | .word        | a-an       |               |
+--------------+--------------+------------+---------------+
|                                                          |
+--------------+--------------+------------+---------------+
| !+           | .word        | a-an       |               |
+--------------+--------------+------------+---------------+
|                                                          |
+--------------+--------------+------------+---------------+
| +!           | .word        | na-        |               |
+--------------+--------------+------------+---------------+
|                                                          |
+--------------+--------------+------------+---------------+
| -!           | .word        | na-        |               |
+--------------+--------------+------------+---------------+
|                                                          |
+--------------+--------------+------------+---------------+
| :is          | .word        | aa-        |               |
+--------------+--------------+------------+---------------+
|                                                          |
+--------------+--------------+------------+---------------+
| :devector    | .word        | aa-        |               |
+--------------+--------------+------------+---------------+
|                                                          |
+--------------+--------------+------------+---------------+
| is           | .word        | a"-        |               |
+--------------+--------------+------------+---------------+
|                                                          |
+--------------+--------------+------------+---------------+
| devector     | .word        | "-         |               |
+--------------+--------------+------------+---------------+
|                                                          |
+--------------+--------------+------------+---------------+
| compile      | .word        | a-         |               |
+--------------+--------------+------------+---------------+
|                                                          |
+--------------+--------------+------------+---------------+
| literal,     | .word        | n-         |               |
+--------------+--------------+------------+---------------+
|                                                          |
+--------------+--------------+------------+---------------+
| tempString   | .word        | $-$        |               |
+--------------+--------------+------------+---------------+
|                                                          |
+--------------+--------------+------------+---------------+
| redraw       | .word        |            |               |
+--------------+--------------+------------+---------------+
|                                                          |
+--------------+--------------+------------+---------------+
| keepString   | .word        | $-$        |               |
+--------------+--------------+------------+---------------+
|                                                          |
+--------------+--------------+------------+---------------+
| getLength    | .word        | $-n        |               |
+--------------+--------------+------------+---------------+
|                                                          |
+--------------+--------------+------------+---------------+
| bye          | .word        |            |               |
+--------------+--------------+------------+---------------+
|                                                          |
+--------------+--------------+------------+---------------+
| (remap-keys) | .word        | c-c        |               |
+--------------+--------------+------------+---------------+
|                                                          |
+--------------+--------------+------------+---------------+
| with-class   | .word        | aa-        |               |
+--------------+--------------+------------+---------------+
|                                                          |
+--------------+--------------+------------+---------------+
| .word        | .word        | a-         |               |
+--------------+--------------+------------+---------------+
|                                                          |
+--------------+--------------+------------+---------------+
| .macro       | .word        | a-         |               |
+--------------+--------------+------------+---------------+
|                                                          |
+--------------+--------------+------------+---------------+
| .data        | .word        | n-         |               |
+--------------+--------------+------------+---------------+
|                                                          |
+--------------+--------------+------------+---------------+
| .inline      | .word        | a-         |               |
+--------------+--------------+------------+---------------+
|                                                          |
+--------------+--------------+------------+---------------+
| .compiler    | .word        | a-         |               |
+--------------+--------------+------------+---------------+
|                                                          |
+--------------+--------------+------------+---------------+
| d->class     | .word        | a-a        |               |
+--------------+--------------+------------+---------------+
|                                                          |
+--------------+--------------+------------+---------------+
| d->xt        | .word        | a-a        |               |
+--------------+--------------+------------+---------------+
|                                                          |
+--------------+--------------+------------+---------------+
| d->name      | .word        | a-a        |               |
+--------------+--------------+------------+---------------+
|                                                          |
+--------------+--------------+------------+---------------+
| boot         | .word        |            |               |
+--------------+--------------+------------+---------------+
|                                                          |
+--------------+--------------+------------+---------------+
| depth        | .word        | -n         |               |
+--------------+--------------+------------+---------------+
|                                                          |
+--------------+--------------+------------+---------------+
| reset        | .word        | ...-       |               |
+--------------+--------------+------------+---------------+
|                                                          |
+--------------+--------------+------------+---------------+
| notfound     | .word        |            |               |
+--------------+--------------+------------+---------------+
|                                                          |
+--------------+--------------+------------+---------------+
| save         | .word        |            |               |
+--------------+--------------+------------+---------------+
|                                                          |
+--------------+--------------+------------+---------------+
| >number      | .word        | $-n        |               |
+--------------+--------------+------------+---------------+
|                                                          |
+--------------+--------------+------------+---------------+
| ok           | .word        |            |               |
+--------------+--------------+------------+---------------+
|                                                          |
+--------------+--------------+------------+---------------+
| s"           | .compiler    | C: "-      |               |
|              |              | R: -$      |               |
+--------------+--------------+------------+---------------+
|                                                          |
+--------------+--------------+------------+---------------+
| [            | .compiler    |            |               |
+--------------+--------------+------------+---------------+
|                                                          |
+--------------+--------------+------------+---------------+
| ;            | .compiler    |            |               |
+--------------+--------------+------------+---------------+
|                                                          |
+--------------+--------------+------------+---------------+
| ;;           | .compiler    |            |               |
+--------------+--------------+------------+---------------+
|                                                          |
+--------------+--------------+------------+---------------+
| =if          | .compiler    | C: -a      |               |
|              |              | R: nn-     |               |
+--------------+--------------+------------+---------------+
|                                                          |
+--------------+--------------+------------+---------------+
| >if          | .compiler    | C: -a      |               |
|              |              | R: nn-     |               |
+--------------+--------------+------------+---------------+
|                                                          |
+--------------+--------------+------------+---------------+
| <if          | .compiler    | C: -a      |               |
|              |              | R: nn-     |               |
+--------------+--------------+------------+---------------+
|                                                          |
+--------------+--------------+------------+---------------+
| !if          | .compiler    | C: -a      |               |
|              |              | R: nn-     |               |
+--------------+--------------+------------+---------------+
|                                                          |
+--------------+--------------+------------+---------------+
| then         | .compiler    | C: a-      |               |
+--------------+--------------+------------+---------------+
|                                                          |
+--------------+--------------+------------+---------------+
| repeat       | .compiler    | C: -a      |               |
+--------------+--------------+------------+---------------+
|                                                          |
+--------------+--------------+------------+---------------+
| again        | .compiler    | C: a-      |               |
+--------------+--------------+------------+---------------+
|                                                          |
+--------------+--------------+------------+---------------+
| 0;           | .compiler    | R: n-      |               |
|              |              | R: n-n     |               |
+--------------+--------------+------------+---------------+
|                                                          |
+--------------+--------------+------------+---------------+
| push         | .compiler    | R: n-      | R: -n         |
+--------------+--------------+------------+---------------+
|                                                          |
+--------------+--------------+------------+---------------+
| pop          | .compiler    | R: -n      | R: n-         |
+--------------+--------------+------------+---------------+
|                                                          |
+--------------+--------------+------------+---------------+
| [']          | .compiler    | C: "-      |               |
|              |              | R:  -n     |               |
+--------------+--------------+------------+---------------+
|                                                          |
+--------------+--------------+------------+---------------+
| for          | .compiler    | C: -a      |               |
|              |              | R: n-      |               |
+--------------+--------------+------------+---------------+
|                                                          |
+--------------+--------------+------------+---------------+
| next         | .compiler    | C: a-      |               |
+--------------+--------------+------------+---------------+
|                                                          |
+--------------+--------------+------------+---------------+
| (            | .macro       | "-         |               |
+--------------+--------------+------------+---------------+
|                                                          |
+--------------+--------------+------------+---------------+
| tx           | .data        | -a         |               |
+--------------+--------------+------------+---------------+
|                                                          |
+--------------+--------------+------------+---------------+
| ty           | .data        | -a         |               |
+--------------+--------------+------------+---------------+
|                                                          |
+--------------+--------------+------------+---------------+
| last         | .data        | -a         |               |
+--------------+--------------+------------+---------------+
|                                                          |
+--------------+--------------+------------+---------------+
| compiler     | .data        | -a         |               |
+--------------+--------------+------------+---------------+
|                                                          |
+--------------+--------------+------------+---------------+
| tib          | .data        | -a         |               |
+--------------+--------------+------------+---------------+
|                                                          |
+--------------+--------------+------------+---------------+
| update       | .data        | -a         |               |
+--------------+--------------+------------+---------------+
|                                                          |
+--------------+--------------+------------+---------------+
| fb           | .data        | -a         |               |
+--------------+--------------+------------+---------------+
|                                                          |
+--------------+--------------+------------+---------------+
| fw           | .data        | -a         |               |
+--------------+--------------+------------+---------------+
|                                                          |
+--------------+--------------+------------+---------------+
| fh           | .data        | -a         |               |
+--------------+--------------+------------+---------------+
|                                                          |
+--------------+--------------+------------+---------------+
| #mem         | .data        | -a         |               |
+--------------+--------------+------------+---------------+
|                                                          |
+--------------+--------------+------------+---------------+
| heap         | .data        | -a         |               |
+--------------+--------------+------------+---------------+
|                                                          |
+--------------+--------------+------------+---------------+
| which        | .data        | -a         |               |
+--------------+--------------+------------+---------------+
|                                                          |
+--------------+--------------+------------+---------------+



