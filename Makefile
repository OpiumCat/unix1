sparse: sparse.c
	gcc -o sparse sparse.c -Wall

clean:
	rm -f sparse