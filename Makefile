mapper: map.c
	gcc -o mapper map.c

reducer: reducer.c
	gcc -o reducer reducer.c

clean:
	rm -rf reducer mapper

all:
	gcc -o mapper map.c
	gcc -o reducer reducer.c
