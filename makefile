main:
	gcc -o shell -g main.c prompt.c commands.c builtin.c userdefined.c history.c nightswatch.c parse.c
