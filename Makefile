output : main.o tsp.o utils.o
	gcc main.o tsp.o utils.o -lm -o main

test : test.o tsp.o utils.o
	gcc test.o tsp.o utils.o -lm -o test

test.o: test.c
	gcc -c test.c

main.o: main.c
	gcc -c main.c

tsp.o: src/tsp.c include/tsp.h
	gcc -c src/tsp.c

utils.o: src/utils.c include/utils.h
	gcc -c src/utils.c

clean:
	rm *.o main test