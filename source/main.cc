//Author: WhiteSheep <260886429@qq.com>
//Created: 2021.8.2
//Description:
#include "WebServer/WebServer.h"
#include "Socket/InetAddress.h"

#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

void daemon_run()
{
	int pid;
	signal(SIGCHLD, SIG_IGN);
	pid = fork();
	if (pid < 0)
	{
		std::cout << "fork error" << std::endl;
		exit(-1);
	}
	else if (pid > 0)
	{
		exit(0);
	}
	setsid();
	int fd;
	fd = open("/dev/null", O_RDWR, 0);
	if (fd != -1)
	{
		dup2(fd, STDIN_FILENO);
		dup2(fd, STDOUT_FILENO);
		dup2(fd, STDERR_FILENO);
	}
	if (fd > 2) {
		close(fd);
	}
}

int main(int argc, char** argv) {
	int threadnumber = 4;
	uint16_t port = 12345;
	int opt;
	const char* optl = "t:p:d:l:s:r";
	bool daemon = false;
	int loglevel = 3;
	int timelimit = 20;
	bool savelog = false;
	while ((opt = getopt(argc, argv, optl)) != -1) {
		switch (opt) {
		case 't':
		{
			threadnumber = atoi(optarg);
			break;
		}
		case 'p':
		{
			port = static_cast<uint16_t>(atoi(optarg));
			break;
		}
		case 'd':
		{
			daemon = true;
			break;
		}
		case 'l':
		{
			loglevel = atoi(optarg);
			break;
		}
		case 's':
		{
			timelimit = atoi(optarg);
			break;
		}
		case 'r':
		{
			savelog = true;
			break;
		}
		default:break;
		}
	}

	if (daemon) {
		daemon_run();
	}

	printf("---------WebServer---------\n");
	printf("pid:\t%d\n", getpid());
	printf("Port:\t%d\n", port);
	printf("Log Level:\t%d\n", loglevel);
	printf("Time Limit:\t%d\n", timelimit);
	printf("Thread Number:\t%d\n", threadnumber);
	InetAddress listenAddr(port);
	WebServer webserver(listenAddr, threadnumber, timelimit, loglevel, savelog);
	webserver.start();
	return 0;
}
