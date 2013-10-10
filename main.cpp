#include <stdio.h>      /* printf, scanf, NULL */
#include <string.h>
#include <ev.h>
#include <glog/logging.h>
#include <Server.hpp>


int main(int argc, char * argv[])
{
	FLAGS_logtostderr = true;
	FLAGS_colorlogtostderr = true;
	google::InitGoogleLogging(argv[0]);
	
	if (argc < 2) {
		// this messsage for user, not for log
		fprintf(stderr, "ERROR, no port provided\n");
		exit(1);
	}
	int port_no = atoi(argv[1]);

	ev::default_loop loop;
	Server::Instance().start(port_no);
	loop.run(0);
	return 0;
}
