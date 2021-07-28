#!/bin/bash

SOURCEFILES=../../src/*.c
OUTPUTFILE=MinimaLogic.o

if [ ! -d "./bin" ]; then
  mkdir bin
fi

if [ ! -d "./bin/linux" ]; then
  mkdir bin/linux
fi

echo -------------------------
echo Building with GCC
echo -------------------------
if ! command -v gcc &> /dev/null
then
  echo GCC Not Found
  echo -------------------------
  exit
fi
pushd bin/linux
gcc $SOURCEFILES -o $OUTPUTFILE -w -Wall -lSDL2main -lSDL2 -lSDL2_ttf
popd
echo -------------------------
echo GCC build Complete
echo -------------------------
