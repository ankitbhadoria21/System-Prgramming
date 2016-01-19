mapper: map.c
	@gcc -o mapper map.c

reducer: reducer.c
	@gcc -o reducer reducer.c

combiner: combiner.c
	@gcc -o combiner combiner.c

clean:
	@rm -rf reducer mapper combiner

all: clean combiner mapper reducer
