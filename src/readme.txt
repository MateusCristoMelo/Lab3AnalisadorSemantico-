
# caveats and changes
# the default name of bison's output is tiny.tab.c and tiny.tab.h, but my cmake sets it to parser.h/c
# my cmake stores flex and bison output files in build. it will not work if other copies of them are in src
# final linking: -ly may be needed in older versions of flex OR without the %option noyywrap 

flex tiny.l
bison -d tiny.y
gcc -c *.c
gcc -o tiny *.o -lfl 

