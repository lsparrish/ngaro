#! /bin/sh
stty -echo -icanon min 1
java retro %1
reset
