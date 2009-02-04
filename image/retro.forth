#! ------------------------------------------------------------
#! Retro, a dialect of Forth
#! Release 10.0.1
#!
#! This code was written by Charles Childers and is gifted to
#! the public domain.
#! ------------------------------------------------------------
   5120 is-data SCRATCH-START
   6144 is-data TIB
   8192 is-data HEAP-START
4000000 is-data STRING-START

begin retroImage
#! ------------------------------------------------------------
mark-dictionary
variable last      ( Pointer to the most recent dictionary )
                   ( header )
HEAP-START
variable: heap     ( Starting address of the data/code heap )
STRING-START variable: STRINGS
#! ------------------------------------------------------------
variable which     ( Pointer to dictionary header of the most )
                   ( recently looked up word )
variable compiler  ( Is the compiler on or off? )
#! ------------------------------------------------------------
variable flag      ( Shared variable used by some primitives )
                   ( It's used as a flag, a counter, etc.    )
#! ------------------------------------------------------------
label: copytag   " RETRO 10" $,
label: nomatch   " Word Not Found" $,
label: okmsg     " ok " $,
#! ------------------------------------------------------------
: dup      dup, ;       : 1+       1+, ;
: 1-       1-, ;        : swap     swap, ;
: drop     drop, ;      : and      and, ;
: or       or, ;        : xor      xor, ;
: @        @, ;         : !        !, ;
: +        +, ;         : -        -, ;
: *        *, ;         : /mod     /mod, ;
: <<       <<, ;        : >>       >>, ;
: out      out, ;       : in       in, ;
: wait     0 # 0 # out, wait, ;

: nip      swap, drop, ;
: over     push, dup, pop, swap, ;
: 2drop    drop, drop, ;
: not      -1 # xor, ;
: rot      push, swap, pop, swap, ;
: -rot     swap, push, swap, pop, ;
: tuck     dup, -rot ;
: 2dup     over over ;
: on       -1 # swap, !, ;
: off       0 # swap, !, ;
: /        /mod, nip ;
: mod      /mod, drop, ;
: neg      -1 # *, ;
: execute   1-, push, ;

: @+ dup, 1+, swap, @, ;
: !+ dup, 1+, push, !, pop, ;
: +! dup, push, @, +, pop, !, ;
: -! dup, push, @, swap, -, pop, !, ;
#! ------------------------------------------------------------
: t-here      ( -a )   heap # @, ;
: t-,         ( n- )   t-here !, t-here 1+, heap # !, ;
: t-]         ( - )    -1 # compiler # !, ;
: t-[         ( - )    0 # compiler # !, ;
: t-;;        ( - )    9 # t-, ;
: t-;         ( - )    t-;; t-[ ;
: ($,)        ( a-a )  repeat dup, @, 0; t-, 1+, again ;
: $           ( a- )   ($,) drop, 0 # t-, ;
: t-push      ( - )    5 # t-, ;
: t-pop       ( - )    6 # t-, ;
: compile     ( a- )   7 # t-, t-, ;
: literal,    ( n- )   1 # t-, t-, ;
: t-for                t-here 5 # t-, ;
: t-next               6 # t-, 27 # t-, 25 # t-, 8 # t-, t-, ;
: t-=if                12 # t-, t-here 0 # t-, ;
: t->if                11 # t-, t-here 0 # t-, ;
: t-<if                10 # t-, t-here 0 # t-, ;
: t-!if                13 # t-, t-here 0 # t-, ;
: t-then               t-here swap, !, 0 # t-, ;
: t-repeat             t-here ;
: t-again              8 # t-, t-, ;
: t-0;                 25 # t-, ;

: .word   ( a- )
  compiler # @, -1 # =if 7 # t-, t-, ; then execute ;
: .inline ( a- )
  dup, @, 0 # !if .word ; then
  compiler # @, -1 # =if 1+, 1+, @, t-, ; then execute ;
: .macro  ( a- )
   execute ;
: .data   ( a- )
   compiler # @, -1 # =if 1 # t-, t-, then ;
: .compiler ( a- )
   compiler # @, 0 # =if drop ; then execute ;

' .word     to 'WORD      ' .macro  to 'MACRO
' .data     to 'DATA      ' .inline to 'INLINE
' .compiler to 'COMPILER
#! ------------------------------------------------------------
variable tx     ( framebuffer text x coordinate  )
variable ty     ( framebuffer text y coordinate  )
variable fb     ( framebuffer addr   )
variable fw     ( framebuffer width  )
variable fh     ( framebuffer height )
-1 variable: update

: redraw update # @, 0; drop, 0 # 3 # out, ;

: fb:cr 0 # tx # !, ty # @, 16 # +, ty # !, ;
: move tx # @, 16 # +, dup, tx # !, fw # @, >if fb:cr then ;
: fb:emit
  dup, 10 # =if fb:cr drop, ; then
  dup, 13 # =if fb:cr drop, ; then
  push, tx # @, ty # @, pop,
  1 # 2 # out, wait move
  redraw
;

: tty:emit 1 # 2 # out, wait redraw ;
: tty:cr   10 # tty:emit ;

: emit   ( c- )  fb # @, 0 # !if fb:emit  ; then tty:emit ;
: cr     ( - )   fb # @, 0 # !if fb:cr    ; then tty:cr ;
: clear  ( - )   -1 # emit 0 # tx # !, 0 # ty # !, ;

: (type) ( a-a ) repeat @+ 0; emit again ;
: type   ( a- )  update # off (type) drop, update # on redraw ;
#! ------------------------------------------------------------
variable >in         ( Offset into the TIB )
variable break-char  ( Holds the delimiter for 'accept' )

: (remap-keys) ;

: key   ( -x )
  repeat
    1 # 1 # out,
    wait 1 # in,
    dup, 0 # !if (remap-keys) ; then drop,
  again
;

: >tib  ( x- )  TIB # >in # @, +, !, ;
: ++    ( - )   1 # >in # +! ;

: (eat-leading)   ( - )
  repeat key dup, emit dup,
         break-char # @, !if >tib ++ ; then drop, again ;

: (accept)        ( -x )
  repeat key dup, emit dup,
         break-char # @, =if ; then >tib ++ again ;

: accept          ( x- )
  break-char # !, 0 # >in # !, (eat-leading) (accept) drop,
  0 # >tib ;
#! ------------------------------------------------------------
: d->class 1+, ;
: d->xt    1+, 1+, ;
: d->name  1+, 1+, 1+, ;

: create     ( "- )  t-here              ( Entry Start )
                     last # @, t-,       ( Link to previous )
                     last # !,           ( Set as newest )
                     'DATA # t-,         ( Class = .data )
                     t-here 0 # t-,      ( XT )
                     32 # accept TIB # $ ( Name )
                     t-here swap, !, ;   ( Patch XT to HERE )
: (:)        ( - )   last # @, d->class !, t-] 0 # t-, 0 # t-, ;
: t-:        ( "- )  create 'WORD  # (:) ;
: t-macro:   ( "- )  create 'MACRO # (:) ;
: t-compiler: ( "- ) create 'COMPILER # (:) ;
: t-(        ( "- )  char: ) # accept ;
#! ------------------------------------------------------------
: n=n        ( xy- )         !if 0 # flag # !, then ;
: get-set    ( ab-xy )       @, swap, @, ;
: next-set   ( ab-a+1 b+1 )  1+, swap, 1+, swap, ;

: (skim)
  repeat
    2dup @, swap, @, +, 0; drop,
    2dup get-set n=n next-set
    flag # @, 0; drop,
  again
;

: compare   ( $1 $2 -- flag )
  -1 # flag # !,
  (skim) 2drop flag # @, ;

: (strlen)
  repeat dup, @, 0; drop, next-set again ;

: getLength ( $1 - n )
  0 # swap, (strlen) drop, ;

variable SAFE
variable LATEST

: (reset-$)  SCRATCH-START # SAFE # !, ;
: (next)     1 # SAFE # +! ;
: (save)     repeat @+ 0; SAFE # @, !, (next) again ;

: tempString
  (reset-$) (save) drop, 0 # SAFE # @, !, SCRATCH-START # ;
: keepString
  STRINGS # @, LATEST # !,
  STRINGS # @, SAFE # !, (save) drop, 0 # SAFE # @, !,
  SAFE # @, 1+, STRINGS # !,
  LATEST # @,
;

: t-" char: " # accept TIB # tempString ;
: t-s" 1 # t-, t-" keepString t-, ;
#! ------------------------------------------------------------
variable #value        variable num
variable #ok           variable negate?


: digit?
 dup, char: 0 # >if dup, char: 9 # <if drop, -1 # ; then then
 drop, 0 # ;

: char>digit char: 0 # -, ;
: digit>char char: 0 # +, ;

: isNegative?
  ( a-a+1 )
  dup, @, char: - # =if -1 # negate? # !, 1+, ; then
  1 # negate? # !, ;

: (convert)
  repeat dup, @, 0; char>digit #value # @, 10 # *, +,
         #value # !, 1+, again ;

: >number
  isNegative? 0 # #value # !, (convert) drop,
  #value # @, negate? # @, *, ;

: (isnumber)
  repeat
    dup, @, 0; digit? flag # @, and, flag # !, 1+,
  again ;

: isnumber?
  isNegative? -1 # flag # !, (isnumber) drop,
  flag # @, ;

: number>digits
  ( x-... )
  1 # flag # +!
  #value # @, 10 # /mod,
  dup, 0 # !if #value # !, jump, ' number>digits , then
  drop, ;

: digits>screen
  ( ...- )
  repeat flag # @, 0; drop, digit>char emit
         flag # @, 1-, flag # !, again ;

: .
  dup, 0 # <if dup, 0 # !if char: - # emit neg then then
  0 # flag # !, #value # !,
  number>digits digits>screen 32 # emit ;
#! ------------------------------------------------------------
variable found

: (search)
  repeat
    dup, d->name TIB # compare
    -1 # =if which # !, found # on ; then
    @, 0;
  again
;

: search
  found # off last # @, (search) ;

: t-'    32 # accept search
         found # @, -1 # =if which # @, d->xt @, ; then 0 #
         found # on ;
: t-[']  1 # t-, t-' t-, ;
#! ------------------------------------------------------------
: :devector dup, 0 # swap, !, 1+, 0 # swap, !, ;
: :is       dup, 8 # swap, !, 1+, !, ;
: devector  t-' :devector ;
: is        t-' :is ;
#! ------------------------------------------------------------
: save 1 # 4 # out, wait ;
: bye jump, MAX-IMAGE , ;
: words
  last # @, repeat dup, d->name type 32 # emit @, 0; again ;
: depth -5 # 5 # out, wait, 5 # in, ;
: reset depth : (reset) 0; push, drop, pop, 1-, (reset) ;
#! ------------------------------------------------------------
variable #mem   ( Amount of memory provided )

: boot 0 # tx # !,  0 # ty # !, copytag # type cr ;
: run-on-boot
  -1 # 5 # out, wait 5 # in, #mem # !,  ( Memory Size )
  -2 # 5 # out, wait 5 # in, fb # !,    ( Framebuffer Addr )
  -3 # 5 # out, wait 5 # in, fw # !,    ( Framebuffer Width )
  -4 # 5 # out, wait 5 # in, fh # !,    ( Framebuffer Height )
  boot ;
#! ------------------------------------------------------------
: with-class ( ac- ) execute ;

: notfound cr nomatch # type cr ;

: the->xt      which # @, d->xt @, ;
: the->class   which # @, d->class @, ;
: tib->number? TIB # isnumber? ;
: tib->number  TIB # >number .data ;

: word?
  found # @, -1 # =if the->xt the->class with-class then ;
: number?
  found # @, 0 # =if tib->number? -1 # =if tib->number ; then
  notfound then ;
: ok compiler # @, 0 # =if cr okmsg # type then ;
: listen  ( - )
  repeat ok 32 # accept search word? number? again ;
#! ----------------------------------------------------------
main:
  run-on-boot listen
#! ----------------------------------------------------------
( Inline )
' 1+           inline: 1+          ' 1-           inline: 1-
' swap         inline: swap        ' drop         inline: drop
' and          inline: and         ' or           inline: or
' xor          inline: xor         ' @            inline: @
' !            inline: !           ' +            inline: +
' -            inline: -           ' *            inline: *
' /mod         inline: /mod        ' <<           inline: <<
' >>           inline: >>          ' nip          inline: nip
' dup          inline: dup         ' in           inline: in
' out          inline: out

( Words )
' t-here       word: here          ' t-,          word: ,
' t-]          word: ]             ' create       word: create
' t-:          word: :             ' t-macro:     word: macro:
' t-compiler:  word: compiler:     ' accept       word: accept
' cr           word: cr            ' emit         word: emit
' type         word: type          ' clear        word: clear
' words        word: words         ' key          word: key
' over         word: over          ' 2drop        word: 2drop
' not          word: not           ' rot          word: rot
' -rot         word: -rot          ' tuck         word: tuck
' 2dup         word: 2dup          ' on           word: on
' off          word: off           ' /            word: /
' mod          word: mod           ' neg          word: neg
' execute      word: execute       ' .            word: .
' t-"          word: "             ' compare      word: compare
' wait         word: wait          ' t-'          word: '
' @+           word: @+            ' !+           word: !+
' +!           word: +!            ' -!           word: -!
' :is          word: :is           ' :devector    word: :devector
' is           word: is            ' devector     word: devector
' compile      word: compile       ' literal,     word: literal,
' tempString   word: tempString    ' redraw       word: redraw
' keepString   word: keepString    ' getLength    word: getLength
' bye          word: bye           ' (remap-keys) word: (remap-keys)
' with-class   word: with-class    ' .word        word: .word
' .macro       word: .macro        ' .data        word: .data
' .inline      word: .inline       ' .compiler    word: .compiler
' d->class     word: d->class      ' d->xt        word: d->xt
' d->name      word: d->name       ' boot         word: boot
' depth        word: depth         ' reset        word: reset
' notfound     word: notfound      ' save         word: save
' >number      word: >number       ' ok           word: ok

( Compiler )
' t-s"         compiler: s"        ' t-[          compiler: [
' t-;          compiler: ;         ' t-;;         compiler: ;;
' t-=if        compiler: =if       ' t->if        compiler: >if
' t-<if        compiler: <if       ' t-!if        compiler: !if
' t-then       compiler: then      ' t-repeat     compiler: repeat
' t-again      compiler: again     ' t-0;         compiler: 0;
' t-push       compiler: push      ' t-pop        compiler: pop
' t-[']        compiler: [']       ' t-for        compiler: for
' t-next       compiler: next

( Macros )
' t-(          macro: (

( Data )
  tx           data: tx            ty           data: ty
  last         data: last          compiler     data: compiler
  TIB          data: tib           update       data: update
  fb           data: fb            fw           data: fw
  fh           data: fh            #mem         data: #mem
  heap         data: heap          which        data: which

patch-dictionary
#! ----------------------------------------------------------
cross-summary
end
