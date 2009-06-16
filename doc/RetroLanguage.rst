The Retro Language
==================
:Author: Charles Childers
:Version: 10.2-dev


===========================
Section 1: Welcome to Retro
===========================

Introduction
------------
Retro is a small, minimalistic dialect of Forth.

The Retro language has a rich history going back to 1998
when it was first released as a 16-bit standalone Forth
for x86 PC's. Since then it's evolved through use into its
present form, which is a 32-bit implementation running on a
portable virtual machine called Ngaro. Today Retro runs on
all major operating systems, and thanks to a JavaScript
implementation of the virtual machine, on any modern web
browser.

The code and documentation for Retro are gifted to the
public domain. You may use them freely and without
restriction.

Platforms Supported
-------------------
- Alpha

  - Linux

- ARM

  - iPhoneOS (iPhone, iPod Touch, 2.2.1; jailbroke)

- PowerPC

  - AIX

- x86

  - BeOS
  - BSD (DragonFly, FreeBSD, NetBSD, OpenBSD)
  - Haiku
  - Linux
  - MacOS X (10.5.x)
  - Windows (XP, Vista)

- x86-64

  - Linux

- Browsers

  - Chrome
  - FireFox (2 & 3)
  - Opera
  - Safari
  - Safari Mobile (using form-based I/O)
  - SongBird

- Java

  - Console
  - MIDP

- .NET (console)

==========================
Section 2: Getting Started
==========================

Obtaining
---------
Retro is generally distributed as source code and needs to be compiled
before it can be used. The latest development snapshots and periodic
stable release snapshots are provided at http://retroforth.org

Developers who want to keep up with the latest changes are encouraged
to use Git. We have a repository at http://github.com/crcx/retro10 which
is kept up to date.

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

::

   make vm

If you get any errors, check the **vm/console/retro.c** and make
sure the **LIBS** line matches the curses varient on your system.

If you are using NetBSD, you can either change the **LIBS** to
**-lcurses** or use the following command to build against ncurses:

::

  make LIBRARY_PATH="/usr/pkg/lib" vm

Or, if you want the framebuffer backend to be used:

::

   make fbvm


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

Building (Java)
---------------
There are two Java implementations. The first, a console based implementation, should
work on all common Unix-like OSes. (Due to poor console API support in Java, it does
not work on Windows) To build this port, ensure that you have the JDK installed and
do:

::

  make java

The second implementation is for MIDP compatible devices. You'll need Ant, the MPP SDK,
and the JDK to build it.

::

  cd vm/midp
  ant

Before building the MIDP version, you should adjust the paths in **build.xml** to match
your systems configuration.

Building (.NET)
---------------
::

  make dotnet


Starting Retro
--------------
How to start depends on which platform you build for. Enter the **bin** directory
and try one of the following:

::

  # Console
  ./retro

  # Framebuffer
  ./retro-fbvm

  # .NET
  mono retro.exe

  # Java
  java retro

If you are using the *console* targets, you can use some command line arguments:

::

  --with filename

Use *filename* as an input source. You can include multiple files by specifying
more than one --with, but you should be aware that the files are included in
reverse order. E.g.,

::

  ./retro --with file-a --with file-b --with file-c

With this, *file-c* is included, then *file-b*, and finally *file-a*.

Interacting with Retro
----------------------
Unlike most Forths, Retro does not buffer on a line by line basis. Input
is parsed as it is typed, and words are executed when the spacebar is
hit.

This is a significant source of confusion to users coming from other
Forth systems. Just remember: only space is recognized by a default
Retro system as a valid separator between words.

Tip:
  Although input is parsed as it is typed, backspace does work on
  most systems, so you can correct the current word being typed if
  you make a mistake.

Leaving Retro
-------------
Just type **bye** and hit space.

Images
------
The Retro language is stored in an image file. When you start Retro,
the VM looks for a **retroImage** file. If if can't find one, it uses
a minimal image that is built in instead.

You can **save** your current Retro session to a retroImage by using
the **save** word, and reload it later. All words/variables you have
created will be kept and you can continue working without having to
reload or retype everything.

You can also use the vector functionality in Retro to replace/alter
most of the existing words to meet your needs.

=========================
Section 3: Implementation
=========================

Overview
--------
Retro is not a standard-compliant Forth. It's significantly
different in many areas. This section will help explain these
differences and show how Retro works internally.

Threading Model
---------------
Retro uses subroutine threading with inline machine code for
select words. This model has been used by Retro since 2001
as it is simple, clean, and allows for optimization to be
done by the compiler if desired.

Taking a look at the subroutine threaded code generated by
Retro:

::

  : foo 1 2 + . ;

Will compile to:

::

  lit 1
  lit 2
  +
  call .
  ;

Simple operations that map to single instructions are
inlined by the Retro compiler, saving some call/return
overhead. Other optimizations are also possible.

Interpreting and Compiling
--------------------------
Retro has a very simple intepreter loop.

::

  : listen  ( - )
    repeat ok 32 # accept search word? number? again ;

This displays a prompt (**ok**), accepts input until a space
is encountered (ASCII 32). The dictionary is searched, and if
the word is found, **word?** calls the class handler for the
word. If not found, **number?** tries to convert it to a
number. If this fails as well, an error is displayed. In any
case, exection repeats until a fatal error arises, or until
the user executes **bye**.

For simplicity sake, the number conversion code in Retro
only supports decimal (base 10). No other base is supported
at this time.

There is no separate compilation process. In Retro, the
**compiler** is nothing more than a state variable that the
*word classes* use to decide what to do with a word.

Word Classes
------------
As mentioned above, the interpreter loop (**listen**) passes
the words (and also data elements like numbers) to something
called a *word class*.

This is another area in which Retro's implementation differs
from standard Forths. The word class approach was created by
Helmar Wodtke and allows for the interpreter and compiler to
be extremely clean by allowing special words (*class handlers*)
to handle different types of words.

This means that the interpreter loop does not need to be
aware of the type a word has, or of any aspect of the system
state.

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

In addition to the five core classes, it is possible to create your
own classes. As an example, we'll create a class for naming and
displaying strings. Our class has the following behavior:

- If interpreting, display the string
- If compiling, lay down the code needed to display the
  string

Retro has a convention of using a . as the first character of a
class name. In continuing this tradition, we'll call our new
class **.string**

Tip:
  On entry to a class, the address of the word or data
  structure is on the stack. The compiler state (which most
  classes will need to check) is in a variable named compiler.

A first step is to lay down a simple skeleton. Since we need to
lay down custom code at compile time, the class handler will
have two parts.

::

  : .string  ( a—)
    compiler @ 0 =if ( interpret time ) ;; then ( compile time )
  ;

We'll start with the interpret time action. We can replace this
with type, since the whole point of this class is to display a
string object.

::

  : .string ( a — )
    compiler @ 0 =if type ;; then ( compile time ) ;

The compile time action is more complex. We need to lay down
the machine code to leave the address of the string on the
stack when the word is run, and then compile a call to type. If
you look at the instruction set listing, you'll see that opcode
1 is the instruction for putting values on the stack. This
opcode takes a value from the following memory location and
puts it on the stack. So the first part of the compile time
action is:

::

  : .string ( a — )
    compiler @ 0 =if type ;; then 1 , , ;

Tip:
  Use **,** to place values directly into memory. This is the
  cornerstone of the entire compiler.

One more thing remains. We still have to compile a call to
type. We can do this by passing the address of type to
compile.

::

  : .string ( a — )
    compiler @ 0 =if type ;; then 1 , , ['] type compile ;

And now we have a new class handler. The second part is to make
this useful. We'll make a creator word called **displayString:** to
take a string and make it into a new word using our .string
class. This will take a string from the stack, make it
permanent, and give it a name.

Tip:
  New dictionary entries are made using create. The class can
  be set after creation by accessing the proper fields in the
  dictionary header. Words starting with **d->** are used to access
  fields in the dictionary headers.

::

  : displayString: ( "name" — )
    create ['] .string last @ d->class ! keepString last @ d->xt ! ;

This uses **create** to make a new word, then sets the class to
**.string** and the xt of the word to the string. It also makes the
string permanent using keepString. last is a variable pointing
to the most recently created dictionary entry. The two words
**d->class** and **d->xt** are dictionary field accessors and are used
to provide portable access to fields in the dictionary.

We can now test the new class:

::

  " hello, world!" displayString: hello
  hello
  : foo hello cr ;
  foo


Vectors
-------
Vectors are another important concept in Retro.

Most Forth systems provide a way to define a word which can
have its meaning altered later. Retro goes a step further by
allowing all words defined using **:** or **macro:** to be
redefined. Words which can be redefined are called *vectors*.

Vectors can be replaced by using **is**, or returned to their
original definition with **devector**. For instance:

::

  : foo 23 . ;
  foo
  : bar 99 . ;
  ' bar is foo
  foo
  devector foo
  foo

There are also variations of **is** and **devector** which take the
addresses of the words rather than parsing for the word name.
These are **:is** and **:devector**.




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
| Returns the next available address on the **heap**       |
+--------------+--------------+------------+---------------+
| ,            | .word        | n-         |               |
+--------------+--------------+------------+---------------+
| Store a byte to the next available address on the heap   |
+--------------+--------------+------------+---------------+
| ]            | .word        |            |               |
+--------------+--------------+------------+---------------+
| Turn **compiler** on                                     |
+--------------+--------------+------------+---------------+
| create       | .word        | "-         |               |
+--------------+--------------+------------+---------------+
| Create a new dictionary header with a class of **.data** |
| and have the address field point to **here**             |
+--------------+--------------+------------+---------------+
| :            | .word        | "-         |               |
+--------------+--------------+------------+---------------+
| Create a new word with a class of **.word** and turn the |
| compiler on                                              |
+--------------+--------------+------------+---------------+
| macro:       | .word        | "-         |               |
+--------------+--------------+------------+---------------+
| Create a new word with a class of **.macro** and turn the|
| compiler on                                              |
+--------------+--------------+------------+---------------+
| compiler:    | .word        | "-         |               |
+--------------+--------------+------------+---------------+
| Create a new word with a class of **.compiler** and turn |
| the compiler on                                          |
+--------------+--------------+------------+---------------+
| accept       | .word        | c-         |               |
+--------------+--------------+------------+---------------+
| Accept input until character *c* is found. Results are   |
| stored in **tib**                                        |
+--------------+--------------+------------+---------------+
| cr           | .word        |            |               |
+--------------+--------------+------------+---------------+
| Emit a newline character                                 |
+--------------+--------------+------------+---------------+
| emit         | .word        | c-         |               |
+--------------+--------------+------------+---------------+
| Display an ASCII character on the screen                 |
+--------------+--------------+------------+---------------+
| type         | .word        | $-         |               |
+--------------+--------------+------------+---------------+
| Display a string on the screen                           |
+--------------+--------------+------------+---------------+
| clear        | .word        |            |               |
+--------------+--------------+------------+---------------+
| Clear the display                                        |
+--------------+--------------+------------+---------------+
| words        | .word        |            |               |
+--------------+--------------+------------+---------------+
| Display a list of all words in the dictionary            |
+--------------+--------------+------------+---------------+
| key          | .word        | -c         |               |
+--------------+--------------+------------+---------------+
| Read a single keypress                                   |
+--------------+--------------+------------+---------------+
| over         | .word        | xy-xyx     |               |
+--------------+--------------+------------+---------------+
| Get a copy of the second item on the stack               |
+--------------+--------------+------------+---------------+
| 2drop        | .word        | xy-        |               |
+--------------+--------------+------------+---------------+
| Drop the top two items from the stack                    |
+--------------+--------------+------------+---------------+
| not          | .word        | x-y        |               |
+--------------+--------------+------------+---------------+
| Logical NOT                                              |
+--------------+--------------+------------+---------------+
| rot          | .word        | xyz-yzx    |               |
+--------------+--------------+------------+---------------+
| Shift the top three values around                        |
+--------------+--------------+------------+---------------+
| -rot         | .word        | xyz-xzy    |               |
+--------------+--------------+------------+---------------+
| **rot** twice                                            |
+--------------+--------------+------------+---------------+
| tuck         | .word        | xy-yxy     |               |
+--------------+--------------+------------+---------------+
| Put a copy of TOS under the second item on the stack     |
+--------------+--------------+------------+---------------+
| 2dup         | .word        | xy-xyxy    |               |
+--------------+--------------+------------+---------------+
| Duplicate the top two items on the stack                 |
+--------------+--------------+------------+---------------+
| on           | .word        | a-         |               |
+--------------+--------------+------------+---------------+
| Set a variable to -1                                     |
+--------------+--------------+------------+---------------+
| off          | .word        | a-         |               |
+--------------+--------------+------------+---------------+
| Set a variable to 0                                      |
+--------------+--------------+------------+---------------+
| /            | .word        | xy-q       |               |
+--------------+--------------+------------+---------------+
| Divide two numbers and get the quotient                  |
+--------------+--------------+------------+---------------+
| mod          | .word        | xy-r       |               |
+--------------+--------------+------------+---------------+
| Divide two numbers and get the remainder                 |
+--------------+--------------+------------+---------------+
| neg          | .word        | x-y        |               |
+--------------+--------------+------------+---------------+
| Invert the sign of x                                     |
+--------------+--------------+------------+---------------+
| execute      | .word        | a-         |               |
+--------------+--------------+------------+---------------+
| Call a word by address                                   |
+--------------+--------------+------------+---------------+
| "            | .word        | "-$        |               |
+--------------+--------------+------------+---------------+
| Parse until " is encountered, returning a string         |
+--------------+--------------+------------+---------------+
| compare      | .word        | $$-f       |               |
+--------------+--------------+------------+---------------+
| Compare two strings for equality                         |
+--------------+--------------+------------+---------------+
| wait         | .word        |            |               |
+--------------+--------------+------------+---------------+
| Wait for an I/O event. Normally used after **out**       |
+--------------+--------------+------------+---------------+
| '            | .word        | "-a        |               |
+--------------+--------------+------------+---------------+
| Parse for a word name and get the address of the word.   |
| Inside a definition use **[']** instead.                 |
+--------------+--------------+------------+---------------+
| @+           | .word        | a-an       |               |
+--------------+--------------+------------+---------------+
| Fetch a value from an address and return the next addrees|
| and the value fetched                                    |
+--------------+--------------+------------+---------------+
| !+           | .word        | na-a       |               |
+--------------+--------------+------------+---------------+
| Store a value to an address and return the next address  |
+--------------+--------------+------------+---------------+
| +!           | .word        | na-        |               |
+--------------+--------------+------------+---------------+
| Add the value n to the contents of address a             |
+--------------+--------------+------------+---------------+
| -!           | .word        | na-        |               |
+--------------+--------------+------------+---------------+
| Subtract the value n from the contents of address a      |
+--------------+--------------+------------+---------------+
| :is          | .word        | aa-        |               |
+--------------+--------------+------------+---------------+
| Change the defintion of a word to call another word. Do  |
| not use with **.data** elements                          |
+--------------+--------------+------------+---------------+
| :devector    | .word        | a-         |               |
+--------------+--------------+------------+---------------+
| Restore the original definition of a word. Not for use   |
| with **.data** elements                                  |
+--------------+--------------+------------+---------------+
| is           | .word        | a"-        |               |
+--------------+--------------+------------+---------------+
| Parse for a name and change its defintion to call the    |
| specified address. Not for use with **.data** elements   |
+--------------+--------------+------------+---------------+
| devector     | .word        | "-         |               |
+--------------+--------------+------------+---------------+
| Parse for a word name and restore it to the original     |
| definition. Not for use with **.data** elements          |
+--------------+--------------+------------+---------------+
| compile      | .word        | a-         |               |
+--------------+--------------+------------+---------------+
| Lay down the code to compile a call to a word            |
+--------------+--------------+------------+---------------+
| literal,     | .word        | n-         |               |
+--------------+--------------+------------+---------------+
| Lay down the code to push a number to the stack          |
+--------------+--------------+------------+---------------+
| tempString   | .word        | $-$        |               |
+--------------+--------------+------------+---------------+
| Move a string to a temporary holding area away from the  |
| **tib**                                                  |
+--------------+--------------+------------+---------------+
| redraw       | .word        |            |               |
+--------------+--------------+------------+---------------+
| If **update** is on, force a screen update. This is used |
| internally to improve performance of I/O operations.     |
+--------------+--------------+------------+---------------+
| keepString   | .word        | $-$        |               |
+--------------+--------------+------------+---------------+
| Move a string to a permanent storage area and return the |
| address                                                  |
+--------------+--------------+------------+---------------+
| getLength    | .word        | $-n        |               |
+--------------+--------------+------------+---------------+
| Return the length of a string                            |
+--------------+--------------+------------+---------------+
| bye          | .word        |            |               |
+--------------+--------------+------------+---------------+
| Exit Retro                                               |
+--------------+--------------+------------+---------------+
| (remap-keys) | .word        | c-c        |               |
+--------------+--------------+------------+---------------+
| Allows for handling and remapping odd key layouts to     |
| something more sane. Called by **key**                   |
+--------------+--------------+------------+---------------+
| with-class   | .word        | aa-        |               |
+--------------+--------------+------------+---------------+
| Call an address using the specified class handler. This  |
| can be revectored to allow tracking statistics or for    |
| debugging purposes                                       |
+--------------+--------------+------------+---------------+
| .word        | .word        | a-         |               |
+--------------+--------------+------------+---------------+
| Class handler for normal words                           |
+--------------+--------------+------------+---------------+
| .macro       | .word        | a-         |               |
+--------------+--------------+------------+---------------+
| Class handler for macros                                 |
+--------------+--------------+------------+---------------+
| .data        | .word        | n-         |               |
+--------------+--------------+------------+---------------+
| Class handler for data elements                          |
+--------------+--------------+------------+---------------+
| .inline      | .word        | a-         |               |
+--------------+--------------+------------+---------------+
| Class handler for simple primitives that can be inlined  |
+--------------+--------------+------------+---------------+
| .compiler    | .word        | a-         |               |
+--------------+--------------+------------+---------------+
| Class handler for compile-time words                     |
+--------------+--------------+------------+---------------+
| d->class     | .word        | a-a        |               |
+--------------+--------------+------------+---------------+
| Given a dictionary header, return the class field        |
+--------------+--------------+------------+---------------+
| d->xt        | .word        | a-a        |               |
+--------------+--------------+------------+---------------+
| Given a dictionary header, return the address field      |
+--------------+--------------+------------+---------------+
| d->name      | .word        | a-a        |               |
+--------------+--------------+------------+---------------+
| Given a dictionary header, return the name field         |
+--------------+--------------+------------+---------------+
| boot         | .word        |            |               |
+--------------+--------------+------------+---------------+
| A hook allowing for custom startup code in an image      |
+--------------+--------------+------------+---------------+
| depth        | .word        | -n         |               |
+--------------+--------------+------------+---------------+
| Return the number of items on the stack                  |
+--------------+--------------+------------+---------------+
| reset        | .word        | ...-       |               |
+--------------+--------------+------------+---------------+
| Drop all items on the stack                              |
+--------------+--------------+------------+---------------+
| notfound     | .word        |            |               |
+--------------+--------------+------------+---------------+
| Called when a word is not found and conversion to a      |
| number fails                                             |
+--------------+--------------+------------+---------------+
| save         | .word        |            |               |
+--------------+--------------+------------+---------------+
| Save the image if the VM supports it, otherwise does     |
| nothing                                                  |
+--------------+--------------+------------+---------------+
| >number      | .word        | $-n        |               |
+--------------+--------------+------------+---------------+
| Try to convert a string to a number                      |
+--------------+--------------+------------+---------------+
| ok           | .word        |            |               |
+--------------+--------------+------------+---------------+
| The "ok" prompt                                          |
+--------------+--------------+------------+---------------+
| listen       | .word        |            |               |
+--------------+--------------+------------+---------------+
| The main interpreter loop                                |
+--------------+--------------+------------+---------------+
| s"           | .compiler    | C: "-      |               |
|              |              | R: -$      |               |
+--------------+--------------+------------+---------------+
| Parse until " is encounterd. Call **keepString** to move |
| the string to the permanent string table, and compile the|
| address of the string into the current definition        |
+--------------+--------------+------------+---------------+
| [            | .compiler    |            |               |
+--------------+--------------+------------+---------------+
| Turn **compiler** off, but don't end the current         |
| definition                                               |
+--------------+--------------+------------+---------------+
| ;            | .compiler    |            |               |
+--------------+--------------+------------+---------------+
| End the current definition and turn **compiler** off     |
+--------------+--------------+------------+---------------+
| ;;           | .compiler    |            |               |
+--------------+--------------+------------+---------------+
| Compile an exit to the word, but do not end the current  |
| definiton                                                |
+--------------+--------------+------------+---------------+
| =if          | .compiler    | C: -a      |               |
|              |              | R: nn-     |               |
+--------------+--------------+------------+---------------+
| Compare two numbers for equality                         |
+--------------+--------------+------------+---------------+
| >if          | .compiler    | C: -a      |               |
|              |              | R: nn-     |               |
+--------------+--------------+------------+---------------+
| Compare two numbers for greater than                     |
+--------------+--------------+------------+---------------+
| <if          | .compiler    | C: -a      |               |
|              |              | R: nn-     |               |
+--------------+--------------+------------+---------------+
| Compare two numbers for less than                        |
+--------------+--------------+------------+---------------+
| !if          | .compiler    | C: -a      |               |
|              |              | R: nn-     |               |
+--------------+--------------+------------+---------------+
| Compare two numbers for inequality                       |
+--------------+--------------+------------+---------------+
| then         | .compiler    | C: a-      |               |
+--------------+--------------+------------+---------------+
| End a conditional                                        |
+--------------+--------------+------------+---------------+
| repeat       | .compiler    | C: -a      |               |
+--------------+--------------+------------+---------------+
| Begin an unconditional loop                              |
+--------------+--------------+------------+---------------+
| again        | .compiler    | C: a-      |               |
+--------------+--------------+------------+---------------+
| End an unconditional loop. Branches back to the last     |
| **repeat**                                               |
+--------------+--------------+------------+---------------+
| 0;           | .compiler    | R: n-      |               |
|              |              | R: n-n     |               |
+--------------+--------------+------------+---------------+
| If TOS is zero, exit the word and drop TOS. Otherwise it |
| leaves TOS alone and continues executing the word. This  |
| is a lightweight control structure borrowed from         |
| HerkForth                                                |
+--------------+--------------+------------+---------------+
| push         | .compiler    | R: n-      | R: -n         |
+--------------+--------------+------------+---------------+
| Move a value from the data stack to the address stack    |
+--------------+--------------+------------+---------------+
| pop          | .compiler    | R: -n      | R: n-         |
+--------------+--------------+------------+---------------+
| Move a value from the address stack to the data stack    |
+--------------+--------------+------------+---------------+
| [']          | .compiler    | C: "-      |               |
|              |              | R:  -n     |               |
+--------------+--------------+------------+---------------+
| Parse for a word name and compile the address of the word|
| into the current definition.                             |
+--------------+--------------+------------+---------------+
| for          | .compiler    | C: -a      |               |
|              |              | R: n-      |               |
+--------------+--------------+------------+---------------+
| Begin a simple counted loop. Takes a count off the stack |
+--------------+--------------+------------+---------------+
| next         | .compiler    | C: a-      |               |
+--------------+--------------+------------+---------------+
| End a simple counted loop. Decrements the counter by 1.  |
| If 0, execute the rest of the word. Otherwise, jumps back|
| to the previous **for**                                  |
+--------------+--------------+------------+---------------+
| (            | .macro       | "-         |               |
+--------------+--------------+------------+---------------+
| Parse until ) is encounterd, ignoring everything. This is|
| used for comments.                                       |
+--------------+--------------+------------+---------------+
| tx           | .data        | -a         |               |
+--------------+--------------+------------+---------------+
| Holds X coordinate for text output (framebuffer only)    |
+--------------+--------------+------------+---------------+
| ty           | .data        | -a         |               |
+--------------+--------------+------------+---------------+
| Holds Y coordinate for text output (framebuffer only)    |
+--------------+--------------+------------+---------------+
| last         | .data        | -a         |               |
+--------------+--------------+------------+---------------+
| Holds the address of the most recent dictionary header   |
+--------------+--------------+------------+---------------+
| compiler     | .data        | -a         |               |
+--------------+--------------+------------+---------------+
| Holds compiler state. 0 if off, -1 if on                 |
+--------------+--------------+------------+---------------+
| tib          | .data        | -a         |               |
+--------------+--------------+------------+---------------+
| The text input buffer                                    |
+--------------+--------------+------------+---------------+
| update       | .data        | -a         |               |
+--------------+--------------+------------+---------------+
| Used by redraw, this allows for caching output to improve|
| performance. Set to 0 if no updates are waiting, or -1   |
| if something is ready to be drawn on the screen.         |
+--------------+--------------+------------+---------------+
| fb           | .data        | -a         |               |
+--------------+--------------+------------+---------------+
| Holds address of framebuffer                             |
+--------------+--------------+------------+---------------+
| fw           | .data        | -a         |               |
+--------------+--------------+------------+---------------+
| Holds width of framebuffer                               |
+--------------+--------------+------------+---------------+
| fh           | .data        | -a         |               |
+--------------+--------------+------------+---------------+
| Holds height of framebuffer                              |
+--------------+--------------+------------+---------------+
| #mem         | .data        | -a         |               |
+--------------+--------------+------------+---------------+
| Holds the amount of memory provided by the VM. This may  |
| or may not include the framebuffer memory, which can be  |
| outside the normal range provided to a Retro image.      |
+--------------+--------------+------------+---------------+
| heap         | .data        | -a         |               |
+--------------+--------------+------------+---------------+
| Holds the address of the top of the heap. This can be    |
| fetched using **here**                                   |
+--------------+--------------+------------+---------------+
| which        | .data        | -a         |               |
+--------------+--------------+------------+---------------+
| Holds the address of the most recently looked up         |
| dictionary header.                                       |
+--------------+--------------+------------+---------------+


==========================
Section 5: Tips and Tricks
==========================

MacOS X
-------
The standard keymaps in the Terminal app don't report the
normal ASCII codes for certain keys. This can be worked around
with **osx.retro**. After building, extend your retroImage:

::

  ./retro --with extras/osx.retro

Save your image, and you'll be able to use backspace in
the future.

Browser
-------
**This is still early, and is not officially supported. The
ports, commands, and return codes may change in the future.**

The JavaScript implementation of the Ngaro VM allows for some
interaction with the browser. With a few simple words you can
quickly take control of the browser (and the VM) by mixing
JavaScript into your Forth code.

::

  : toggle-html ( - ) 1 1024 out wait ;

When invoked, this will toggle the filtering of special characters
by the console driver on and off. (By default the special
characters are filtered. These include < > & and others). When
the filter is off, you can use HTML to format the output in the
console.

::

  : js ( $- ) 2 1024 out wait ;

This is the more powerful of the two. It allows for passing a
Retro string to the JavaScript eval() function. You can pass
any valid JavaScript code and have it run. You can also access
the variables and functions of the Ngaro VM using it.

Something simple to try:

::

  : depth s" alert(sp);" js ;

Framebuffer
-----------
**This is still early, and is not officially supported. The
ports, commands, and return codes may change in the future.**

The framebuffer backed VM has some limited support for using a
mouse.

::

  : get-coords ( -xy ) -1 12 out wait ;

==========================
Section 6: Core Extensions
==========================

Overview
--------
While working with Retro, I've assembled a set of extensions that
I personally find useful, but which don't need to be in the core.


Loading the Extensions
----------------------
The extensions are provided in a single file named **extend.retro**.
This can be found in the **bin** directory. Load it by doing:

::

  ./retro --with extras/extend.retro

Save, and the extensions will remain present in future sessions.


The Words
---------
The main words provided by **extend.retro** are covered in the list
below. Some of the internal factors are not listed.


+--------------+--------------+------------+---------------+
| Name         | Class        | Data Stack | Address Stack |
+==============+==============+============+===============+
| {            | .word        |            |               |
+--------------+--------------+------------+---------------+
| Start a local namespace                                  |
+--------------+--------------+------------+---------------+
| }            | .word        |            |               |
+--------------+--------------+------------+---------------+
| Close a local namespace                                  |
+--------------+--------------+------------+---------------+
| {{           | .word        |            |               |
+--------------+--------------+------------+---------------+
| Start a mixed namespace                                  |
+--------------+--------------+------------+---------------+
| ---reveal--- | .word        |            |               |
+--------------+--------------+------------+---------------+
| Switch to global namespace                               |
+--------------+--------------+------------+---------------+
| }}           | .word        |            |               |
+--------------+--------------+------------+---------------+
| Close a mixed namespace                                  |
+--------------+--------------+------------+---------------+
| allot        | .word        | n-         |               |
+--------------+--------------+------------+---------------+
| Allocate n cells of data                                 |
+--------------+--------------+------------+---------------+
| variable:    | .word        | n"-        |               |
+--------------+--------------+------------+---------------+
| Create a variable with an initial value of n             |
+--------------+--------------+------------+---------------+
| variable     | .word        | "-         |               |
+--------------+--------------+------------+---------------+
| Create a variable with an initial value of 0             |
+--------------+--------------+------------+---------------+
| constant     | .word        | n"-        |               |
+--------------+--------------+------------+---------------+
| Create a constant with a value of n                      |
+--------------+--------------+------------+---------------+
| ++           | .word        | a-         |               |
+--------------+--------------+------------+---------------+
| Increment the value of a variable                        |
+--------------+--------------+------------+---------------+
| --           | .word        | a-         |               |
+--------------+--------------+------------+---------------+
| Decrement a variable                                     |
+--------------+--------------+------------+---------------+
| copy         | .word        | aan-       |               |
+--------------+--------------+------------+---------------+
| Copy n cells from source to dest                         |
+--------------+--------------+------------+---------------+
| fill         | .word        | ann-       |               |
+--------------+--------------+------------+---------------+
| Takes an address, a value, and a count and fills count   |
| cells of memory starting at address with a value         |
+--------------+--------------+------------+---------------+
| `\``         | .compiler    | "-         |               |
+--------------+--------------+------------+---------------+
| Replaces these forms:                                    |
| ::                                                       |
|                                                          |
| ` wordname  =  ['] wordname compile                      |
| ` wordname  =  ['] wordname execute                      |
+--------------+--------------+------------+---------------+
| ."           | .macro       | "-         |               |
+--------------+--------------+------------+---------------+
| Parse until " and display the string. If compiling, lay  |
| down the code to display the string.                     |
+--------------+--------------+------------+---------------+
| TRUE         | .word        | -f         |               |
+--------------+--------------+------------+---------------+
| Return -1                                                |
+--------------+--------------+------------+---------------+
| FALSE        | .word        | -f         |               |
+--------------+--------------+------------+---------------+
| Return 0                                                 |
+--------------+--------------+------------+---------------+
| if           | .compiler    | C: -a      |               |
|              |              | R: f-      |               |
+--------------+--------------+------------+---------------+
| Start a conditional. Execute if flag is TRUE             |
+--------------+--------------+------------+---------------+
| ;then        | .compiler    | C: a-      |               |
+--------------+--------------+------------+---------------+
| Same as **;; then**                                      |
+--------------+--------------+------------+---------------+
| =            | .word        | nn-f       |               |
+--------------+--------------+------------+---------------+
| Check for equality                                       |
+--------------+--------------+------------+---------------+
| >            | .word        | nn-f       |               |
+--------------+--------------+------------+---------------+
| Check for greater than                                   |
+--------------+--------------+------------+---------------+
| <            | .word        | nn-f       |               |
+--------------+--------------+------------+---------------+
| Check for less than                                      |
+--------------+--------------+------------+---------------+
| <>           | .word        | nn-f       |               |
+--------------+--------------+------------+---------------+
| Check for inequality                                     |
+--------------+--------------+------------+---------------+
| forget       | .word        | "-         |               |
+--------------+--------------+------------+---------------+
| Parse a word name, and remove that word and all words    |
| defined after it from memory                             |
+--------------+--------------+------------+---------------+



=======================
Section 7: Block Editor
=======================

Introduction
------------
Retro ships with a small block editor. This is based on a
series of earlier editors, and has a few nice features:

- Blocks are stored in the image
- External tools allow extracting and moving blocks to a
  new image
- All editing words are vectors allowing more control over
  the editor

To load it:

::

  ./retro --with extras/editor.retro --with extras/extend.retro


Tip:
  Line and column numbers start at 0

The Words
---------
+--------------+-------------------------------------+
| Usage        | Description                         |
+==============+=====================================+
|     # s      | Select a new block                  |
+--------------+-------------------------------------+
|       p      | Previous block                      |
+--------------+-------------------------------------+
|       n      | Next block                          |
+--------------+-------------------------------------+
|     # i ..   | Insert .. into line                 |
+--------------+-------------------------------------+
|  # #2 ia ..  | Insert .. into line [#2] starting at|
|              | column [#]                          |
+--------------+-------------------------------------+
|      x       | Erase the current block             |
+--------------+-------------------------------------+
|    # d       | Erase the specified line            |
+--------------+-------------------------------------+
|      v       | Display the current block           |
+--------------+-------------------------------------+
|      e       | Evaluate Block                      |
+--------------+-------------------------------------+
|      new     | Erase all blocks                    |
+--------------+-------------------------------------+



=====================
Section 8: Retrospect
=====================

Introduction
------------
Retrospect is a debugging aid. It provides a decompiler,
memory dumper, and other tools that may prove useful in
better understanding your code and the compiler.

To load it:

::

  ./retro --with extras/retrospect.retro --with extras/extend.retro

Retrospect requires the extend.retro package to be loaded
before it will work.


The Words
---------
+--------------+--------------+------------+---------------+
| Name         | Class        | Data Stack | Address Stack |
+==============+==============+============+===============+
| dump         | .word        | an-        |               |
+--------------+--------------+------------+---------------+
| Dump n values from the memory location starting at a     |
+--------------+--------------+------------+---------------+
| show         | .word        | an-        |               |
+--------------+--------------+------------+---------------+
| Decompile n instructions starting at address a           |
+--------------+--------------+------------+---------------+
| see          | .word        | "-         |               |
+--------------+--------------+------------+---------------+
| Decompile a word back to source. Parses for a word name, |
| then attempts to detect the end of the word. May display |
| the dictionary header for the following word as well.    |
+--------------+--------------+------------+---------------+
