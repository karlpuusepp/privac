all:
	gcc privac.c -lncurses -lwebsockets -o priva_c
osx:
	gcc privac.c -lncurses -L/usr/local/lib -lwebsockets -o priva_c
