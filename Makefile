myshell: myshell.c
	gcc -Wall -g -Werror -o myshell myshell.c

clean:
	rm -f myshell
