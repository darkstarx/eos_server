#include <stdio.h>      /* printf, scanf, NULL */
#include <stdlib.h>     /* malloc, free, rand */
#include <string.h>
#include <iostream>
#include <unistd.h>
#include <ev.h>
#include <Server.hpp>


int main(int argc, char *argv[])
{
	if (argc < 2) {
		fprintf(stderr, "ERROR, no port provided\n");
		exit(1);
	}
	int port_no = atoi(argv[1]);

	ev::default_loop loop;
	Server::Instance().start(port_no);
	loop.run(0);
	return 0;
}
