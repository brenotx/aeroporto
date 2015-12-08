.PHONY: ALL clean fclean

ALL: AEROPORTO clean

AEROPORTO: estruturas.o main.c
	gcc -std=gnu99 -Wall estruturas.o main.c -o aeroporto -pthread

estruturas.o: estruturas.c estruturas.h
	gcc -std=gnu99 -Wall estruturas.c estruturas.h -c -pthread
	
clean:
	rm -f *.gch
	rm -f *.o

fclean: clean
	rm -f aeroporto
