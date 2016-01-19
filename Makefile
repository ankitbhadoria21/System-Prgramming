map: map.c
	gcc -o map map.c

reducer: reducer.c
	gcc -o reducer reducer.c

clean:
	rm -rf reducer map

all:
	gcc -o map map.c
	gcc -o reducer reducer.c
