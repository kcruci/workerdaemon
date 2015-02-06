#!/bin/sh

find $PWD -name "*.h" -o -name "*.c" -o -name "*.cc" -o -name "*.cpp" -o -name "*.hpp" > cscope.files
cscope -bkq -i cscope.files
ctags -R
