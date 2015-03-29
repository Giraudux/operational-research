# Alexis Giraudet

CC=gcc
SRC_IN=robots.c
EXEC_OUT=robots
NB_PROC=2

all:
	$(CC) -I./local/include -L./local/lib -O3 -ansi -o ./$(EXEC_OUT) ./$(SRC_IN) -lglpk -lm

debug:
	$(CC) -I./local/include -L./local/lib -DNDEBUG -g -Wall -ansi -o ./$(EXEC_OUT) ./$(SRC_IN) -lglpk -lm

test:
	./$(EXEC_OUT) ./plat/exemple.dat

glpk:
	-mkdir -p ./local
	tar -xf ./glpk-4.55.tar.gz && \
	cd glpk-4.55 && \
	CC=$(CC) CFLAGS=-O3 ./configure --prefix=$(shell pwd)/local --disable-shared && \
	make && \
	make install

resolve-all-plat:
	find ./plat/ -name '*.dat' -print0 | xargs -I file -0 -P $(NB_PROC) bash -c "echo Début de la résolution de file; ./$(EXEC_OUT) file > file.txt; echo Fin de la résolution de file"

resolve-all-relief:
	find ./relief/ -name '*.dat' -print0 | xargs -I file -0 -P $(NB_PROC) bash -c "echo Début de la résolution de file; ./$(EXEC_OUT) file > file.txt; echo Fin de la résolution de file"

resolve-all:
	find ./plat/ ./relief/ -name '*.dat' -print0 | xargs -I file -0 -P $(NB_PROC) bash -c "echo Début de la résolution de file; ./$(EXEC_OUT) file > file.txt; echo Fin de la résolution de file"

clean:
	-rm -r ./glpk-4.55
	-rm -r ./local
	-rm ./$(EXEC_OUT)
