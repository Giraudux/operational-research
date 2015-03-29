# Alexis Giraudet

CC=gcc

all:
	$(CC) -I./local/include -L./local/lib -O3 -ansi -o ./robots ./robots.c -lglpk -lm

debug:
	$(CC) -I./local/include -L./local/lib -DNDEBUG -g -Wall -ansi -o ./robots ./robots.c -lglpk -lm

test:
	./robots ./plat/exemple.dat

glpk:
	-mkdir -p ./local
	tar -xf ./glpk-4.55.tar.gz && \
	cd glpk-4.55 && \
	CC=$(CC) CFLAGS=-O3 ./configure --prefix=$(shell pwd)/local --disable-shared && \
	make && \
	make install

resolve-all-plat:
	-mkdir -p ./solution/plat

resolve-all-relief:
	-mkdir -p ./solution/relief

resolve-all: resolve-all-plat resolve-all-relief

clean:
	-rm -r ./glpk-4.55
	-rm -r ./local
	-rm -r ./solution
	-rm ./robots
	-rm ./robots.lp
