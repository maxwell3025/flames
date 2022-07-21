flames: main.o solver.o field.o
	cc -o flames main.o solver.o field.o -lncurses -lm
main.o: main.c solver.h field.h
	cc -c main.c
solver.o: solver.c solver.h field.h
	cc -c solver.c
field.o: field.c field.h
	cc -c field.c
clean:
	rm flames main.o solver.o field.o
run: flames
	./flames
install: flames
	sudo cp flames /usr/local/bin/flames
