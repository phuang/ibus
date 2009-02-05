#!/bin/sh
codegendir=`pkg-config pygobject-2.0 --variable=codegendir`
python $codegendir/h2def.py -m ibus ../../src/*.h
