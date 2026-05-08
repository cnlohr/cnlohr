all : msdfgen2

msdfgen2 : msdfgen2.c
	gcc -o $@ $^ -lm -fsanitize=address -Og -g

clean :
	rm -rf msdfgen2

