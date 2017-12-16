clientservermake: code.c
	gcc -lpthread -o run code.c 
run: run
	./run
