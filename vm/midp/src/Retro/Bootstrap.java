package Retro;

/**********************************************************************
 * RETRO 10 J2ME
 * Written by Martin Polak
 * Released into the public domain.
 *
 **********************************************************************/

public class Bootstrap extends Object {
  public String [] boot = new String [100];

  public Bootstrap() {
    boot[0] = ": allot  ( n -- )  heap +! ;";
    boot[1] = ": variable: create , ;";
    boot[2] = ": variable 0 variable: ;";
    boot[3] = ": constant create last @ d->xt ! ;";
    boot[4] = ": ++  ( a -- )  1 swap +! ;";
    boot[5] = ": --  ( a -- )  1 swap -! ;";
    boot[6] = "variable src";
    boot[7] = "variable dst";
    boot[8] = ": (copy) for src @ @ dst @ ! src ++ dst ++ next ;";
    boot[9] = ": copy push dst ! src ! pop (copy) ;";
    boot[10] = "variable dst";
    boot[11] = "variable val";
    boot[12] = ": (fill) for val @ dst @ ! dst ++ next ;";
    boot[13] = ": fill push val ! dst ! pop (fill) ;";
    boot[14] = "macro: `c ' literal, ['] compile compile ;";
    boot[15] = "macro: `x ' compile ;";
    boot[16] = "macro: .\" ";
    boot[17] = "   compiler @ 0 =if \" type ;; then `x s\" `c type ;";
    boot[18] = "-1 constant TRUE   ( -- flag )";
    boot[19] = " 0 constant FALSE  ( -- flag )";
    boot[20] = "macro: if    ( flag -- )";
    boot[21] = "       compiler @ 0; drop TRUE literal, `x =if ;";
    boot[22] = "macro: ;then ( -- )";
    boot[23] = "       compiler @ 0; drop `x ;; `x then ;";
    boot[24] = ": =  ( x y -- flag )  =if TRUE ;then FALSE ;";
    boot[25] = ": <> ( x y -- flag )  !if TRUE ;then FALSE ;";
    boot[26] = ": >  ( x y -- flag )  >if TRUE ;then FALSE ;";
    boot[27] = ": <  ( x y -- flag )  <if TRUE ;then FALSE ;";
    boot[28] = " 48 constant #-blocks";
    boot[29] = " 256 constant #-block-size";
    boot[30] = "32000 #-block-size #-blocks * - constant offset";
    boot[31] = "167 variable: line-ending";
    boot[32] = "variable blk";
    boot[33] = ": block     256 * offset + ;";
    boot[34] = ": (block)   blk @ block ;";
    boot[35] = ": (line)    32 * (block) + ;";
    boot[36] = ": (type)  for dup @ emit 1+ next ;";
    boot[37] = ": type    (type) drop ;";
    boot[38] = ": space   32 emit ;";
    boot[39] = ": row     . dup 32 type 32 + cr ;";
    boot[40] = ": .rows   0 row 1 row 2 row 3 row";
    boot[41] = "          4 row 5 row 6 row 7 row ;";
    boot[42] = ": .block  .\" Block: \" blk @ . .\" of \" #-blocks . ;";
    boot[43] = ": xbar    .\" +---:---+---:---\" ;";
    boot[44] = ": bar     space space xbar xbar cr ;";
    boot[45] = ": vb      blk @ block .rows drop bar ;";
    boot[46] = ": status  .block ;";
    boot[47] = ": (v) clear vb status ;";
    boot[48] = "variable count";
    boot[49] = "variable buffer";
    boot[50] = ": setup  ( -- )    256 count ! (block) buffer ! ;";
    boot[51] = ": -remap ( -- )    ['] key :devector ;";
    boot[52] = ": get buffer @ @ ;";
    boot[53] = ": nxt ";
    boot[54] = "  count @ 0 =if 32 -remap ;then";
    boot[55] = "  count -- get buffer ++ ;";
    boot[56] = ": remap  ( -- )    ['] nxt ['] key :is ;";
    boot[57] = ": (e) setup remap ;";
    boot[58] = ": v   ( -- ) (v) ;";
    boot[59] = ": s   ( block -- ) blk ! v ;";
    boot[60] = ": d   ( line -- )  (line) 32 32 fill v ;";
    boot[61] = ": x   ( -- )       (block) 32 256 fill v ;";
    boot[62] = ": p   ( -- )       -1 blk +! v ;";
    boot[63] = ": n   ( -- )        1 blk +! v ;";
    boot[64] = ": ia ";
    boot[65] = "  (line) + push line-ending @ accept tib pop tib getLength copy v ;";
    boot[66] = ": i 0 swap ia v ;";
    boot[67] = ": new ( -- ) offset 32 #-block-size #-blocks * fill ;";
    boot[68] = ": e   ( -- ) (e) ;";
    boot[69] = "new";
  }
}
