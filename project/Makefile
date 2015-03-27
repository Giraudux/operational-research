# Alexis Giraudet

all:
	gcc -I./local/include -L./local/lib -O3 -ansi -o ./robots ./robots.c -lglpk -lm

debug:
	gcc -I./local/include -L./local/lib -DNDEBUG -g -Wall -ansi -o ./robots ./robots.c -lglpk -lm

test:
	./robots ./plat/exemple.dat

glpk:
	wget http://ftp.gnu.org/gnu/glpk/glpk-4.55.tar.gz && \
	tar -xf ./glpk-4.55.tar.gz && \
	cd glpk-4.55 && \
	mkdir local && \
	CFLAGS=-O3 ./configure --prefix=$(shell pwd)/local --disable-shared && \
	make && \
	make install

clean:
	-rm -r ./glpk-4.55*
	-rm -r ./local
	-rm ./robots
	-rm ./robots.lp
