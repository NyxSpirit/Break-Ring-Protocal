run: main.o rsm.o rpkg.o testenv.o log.o						
	gcc -o run main.o rsm.o rpkg.o testenv.o log.o -lpthread
main.o: main.c
	gcc -c main.c
rsm.o: rsm.c
	gcc -c rsm.c -lpthread 
rpkg.o: rpkg.c
	gcc -c rpkg.c
testenv.o: testenv.c
	gcc -c testenv.c
log.o: log.c
	gcc -c log.c

clean:
	rm -f *.o run
