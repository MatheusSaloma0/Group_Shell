all:
	gcc input_manager.c operations.c main.c -o gsh
run:
	./gsh