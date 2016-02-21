all:
	@gcc sem.c phil.c -o phil -lpthread
	@gcc init.c sem.c -o host -lpthread

clean:
	@rm -rf phil host *.o
