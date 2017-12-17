clientservermake: code.c
	gcc -lrt -lpthread -o run code.c 
run: run
	./run
debug: code.c
	gcc -g -lrt -lpthread -o run code.c
