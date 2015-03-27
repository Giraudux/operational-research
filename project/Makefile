# Alexis Giraudet

all: static

shared:
	gcc -O2 -ansi -o ./robots ./robots.c -lglpk -lm

static: glpk
	gcc -I./local/include -O2 -ansi -o ./robots ./robots.c ./local/lib/libglpk.a -lm

test:
	./robots ./plat/exemple.dat

glpk:
	wget http://ftp.gnu.org/gnu/glpk/glpk-4.55.tar.gz && \
	tar -xf ./glpk-4.55.tar.gz && \
	cd glpk-4.55 && \
	mkdir local && \
	CFLAGS=-O2 ./configure --prefix=$(shell pwd)/local --disable-shared && \
	make && \
	make install

clean:
	-rm -r ./glpk-4.55*
	-rm -r ./local
	-rm ./robots
	-rm ./robots.lp
