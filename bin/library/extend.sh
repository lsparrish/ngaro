#! /bin/sh
#! ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#! Tool for building extended Retro images quickly
#! ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
if [ $1 ]
then
  if [ $1 == "--new" ]
  then
    rm -f retroImage
    cd standard
    cat extend.retro retrospect.retro quotes.retro editor.retro >../bootstrap.retro
    cd ..
    echo "save bye" >>bootstrap.retro
    ../retro --with bootstrap.retro
    rm -f bootstrap.retro
  fi
  if [ $1 == "--add" ]
  then
    for var in $9 $8 $7 $6 $5 $4 $3 $2
    do
      cat $var >>bootstrap.retro
    done
    echo "save bye" >>bootstrap.retro
    ../retro --with bootstrap.retro
    rm -f bootstrap.retro
  fi
  if [ $1 == "--clean" ]
  then
    rm -f retroImage bootstrap.retro
  fi
else
  echo "-------------------------------------------------------------"
  echo "extend.sh - tool for adding extensions to a Retro image"
  echo " "
  echo "./extend.sh --new"
  echo "Create a new retroImage with the core extensions loaded."
  echo " "
  echo "./extend.sh --add file1 file2 ... file8"
  echo "Load files into a Retro image. Can accept up to 8 filenames."
  echo " "
  echo "./extend.sh --clean"
  echo "Remove retroImage and any temporary files created"
  echo "-------------------------------------------------------------"
fi
