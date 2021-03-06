#   -*- mode: makefile; -*-

GCC_WFLAGS=	-Wall -Weffc++ -pedantic  \
    -pedantic-errors -Wextra -Wcast-align \
    -Wcast-qual  -Wchar-subscripts  -Wcomment -Wconversion \
    -Wdisabled-optimization \
    -Werror -Wfloat-equal  -Wformat  -Wformat=2 \
    -Wformat-nonliteral -Wformat-security  \
    -Wformat-y2k \
    -Wimport  -Winit-self  -Winline \
    -Winvalid-pch   \
    -Wunsafe-loop-optimizations -Wmissing-braces \
    -Wmissing-field-initializers -Wmissing-format-attribute   \
    -Wmissing-include-dirs -Wmissing-noreturn \
    -Wpacked  -Wparentheses  -Wpointer-arith \
    -Wredundant-decls -Wreturn-type \
    -Wsequence-point  -Wshadow -Wsign-compare  -Wstack-protector \
    -Wstrict-aliasing -Wstrict-aliasing=2 -Wswitch  -Wswitch-default \
    -Wswitch-enum -Wtrigraphs  -Wuninitialized \
    -Wunknown-pragmas  -Wunreachable-code -Wunused \
    -Wunused-function  -Wunused-label  -Wunused-parameter \
    -Wunused-value  -Wunused-variable  -Wvariadic-macros \
    -Wvolatile-register-var  -Wwrite-strings \
		-ftrapv \
		-Werror=implicit-function-declaration

GCC_FLAGS=	-D_FORTIFY_SOURCE=2 -D_GLIBCXX_ASSERTIONS \
		-fasynchronous-unwind-tables -fexceptions -fpie -Wl,-pie \
		-fstack-clash-protection -fstack-protector-all \
		-g -grecord-gcc-switches -fcf-protection -O2 -pipe \
		-Wl,-z,defs -Wl,-z,now -Wl,-z,relro \
		-std=c++2a

CPPFLAGS+=	${GCC_FLAGS} ${GCC_WFLAGS}

CLANGBIN=		clang++-8
CLANGFLAGS= -Wall -std=c++2a -O2

CLANG=			${CLANGBIN} ${CLANGFLAGS}

all: main

clean:
		rm -f main main2 main3

ifneq (,$(wildcard ./main2.cpp))
all: main2

clang: main2.cpp
endif

clang: main.cpp
		${CLANG} -o main main.cpp
ifneq (,$(wildcard ./main2.cpp))
		${CLANG} -o main2 main2.cpp
endif
