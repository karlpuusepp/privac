all:
	gcc privac.c -lncurses -lwebsockets -o priva_c
	gcc privac-server.c -lwebsockets -o privac-server
osx:
	gcc privac.c -lncurses -L/usr/local/lib -lwebsockets -DOSX=1 -o priva_c 
	gcc privac-server.c -L/usr/local/lib -lwebsockets -DOSX=1 -o privac-server
