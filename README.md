privac
--------
A small text-based chat client and server written in C using libwebsockets and ncurses as a project for a "programming in C" class.

The client and server have minimal functionality and error checking, so it's basically proof-of-concept of a singlethreaded TTY chat app with client-side XOR crypto written in gcc inline asm. 

When running the client, make sure to pipe stderr to a logfile or /dev/null, otherwise it'll break ncurses.

(c) rerelease, 2013. all rights reserved, all wrongs reversed.
