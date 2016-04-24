run: main.o rsm.o rpkg.o						
	gcc -o run main.o rsm.o rpkg.o
main.o: main.c
	gcc -c main.c
rsm.o: rsm.c
	gcc -c rsm.c
rpkg.o: rpkg.c
	gcc -c rpkg.c

clean:
	rm -f *.o run
