/*
 * PyTblinkHdlLauncher.cpp
 *
 *  Created on: Jul 8, 2021
 *      Author: mballance
 */

#include <string.h>
#include "PyTblinkHdlLauncher.h"
#include "tblink_rpc/tblink_rpc.h"

#ifndef _WIN32
#include <pthread.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/stat.h>
#include <fcntl.h>
static const char PS = ':';
#else
#include <winsock2.h>
static const char PS = ';';
#endif
#include <spawn.h>
#include <string.h>

extern char **environ;

PyTblinkHdlLauncher::PyTblinkHdlLauncher() {
	// TODO Auto-generated constructor stub

}

PyTblinkHdlLauncher::~PyTblinkHdlLauncher() {
	// TODO Auto-generated destructor stub
}

IEndpoint *PyTblinkHdlLauncher::launch(
			IEndpointServices		*services) {
	bool ret = true;
	std::string python;
	IFactory *factory = tblink_rpc_get_factory();

	// First, locate a Python interpreter
	const char *tblink_python = getenv("TBLINK_PYTHON");

	if (tblink_python && tblink_python[0]) {
		python = tblink_python;
	} else {
		python = find_python();
	}

	if (python == "") {
		fprintf(stdout, "Error: failed to find Python\n");
		return 0;
	}

	// Create the socket server
	struct sockaddr_in serv_addr;

	int32_t srv_socket = socket(AF_INET, SOCK_STREAM, 0);

    memset(&serv_addr, 0, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    serv_addr.sin_port = 0;

    if ((bind(srv_socket, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) < 0) {
    	perror("Error binding");
    }

    socklen_t size = sizeof(serv_addr);
    getsockname(srv_socket, (struct sockaddr *) &serv_addr, &size);

    fprintf(stdout, "port: %d\n", serv_addr.sin_port);
    fflush(stdout);
//    cd.port = serv_addr.sin_port;
//    ASSERT_EQ(pthread_create(&thread, 0, &client_f, &cd), 0);

    listen(srv_socket, 1);

	// Spawn the interpreter
   	pid_t pid;
    {
    	std::vector<std::string> args;
    	char tmp[16], hostname[16], m[4], module[16];
    	char *exec_t = (char *)alloca(python.size()+1);

//    	strcpy(hostname, "localhost");
    	strcpy(m, "-m");
    	strcpy(module, "tblink_rpc_hdl.runtime");
    	strcpy(exec_t, python.c_str());

    	sprintf(tmp, "%d", ntohs(serv_addr.sin_port));
    	char **argv = (char **)alloca(sizeof(char *)*6);
    	argv[0] = exec_t;
    	argv[1] = m;
    	argv[2] = module;
 //   	argv[3] = hostname;
    	argv[3] = tmp;
    	argv[4] = 0;

    	int status = posix_spawnp(&pid, argv[0], 0, 0, (char *const *)argv, environ);

    	if (status != 0) {
    		fprintf(stdout, "Error: Failed to launch python \"%s\"\n", python.c_str());
    		return 0;
    	}
    }

	// Wait for a connection, or for
	// the interpreter process to exit
   	int32_t conn_socket = -1;
    {
    	// Wait for a bit to see if we get a connect request
    	fd_set rfds;
    	struct timeval tv;
    	int retval;


    	/*
    	 * Wait for a connect request for 10ms
    	 * at a time. * Wait up to five seconds total
    	 */
    	for (uint32_t i=0; i<500; i++) {
    		tv.tv_sec = 0;
    		tv.tv_usec = 10000;

    		FD_ZERO(&rfds);
    		FD_SET(srv_socket, &rfds);

    		retval = select(srv_socket+1, &rfds, 0, 0, &tv);

    		if (retval > 0) {
    			break;
    		} else {
    			// Check that the process is still alive
    			int status;
    			retval = ::waitpid(pid, &status, WNOHANG);

    			// If the process either doesn't exist
    			// or has terminated, then bail out early
    			if (retval != 0) {
    				fprintf(stdout, "Note: Python process has terminated\n");
    				retval = -1;
    				break;
    			}
    		}
    	}

    	if (retval > 0) {
    		unsigned int clilen = sizeof(serv_addr);
    		conn_socket = accept(srv_socket, (struct sockaddr *)&serv_addr, &clilen);
    	}
    }

    if (conn_socket == -1) {
    	fprintf(stdout, "Error: accept failed\n");
    	return 0;
    }

    fprintf(stdout, "Note: socket %d\n", conn_socket);

#ifdef _WIN32
   unsigned long mode = 0;
   ioctlsocket(m_conn_socket, FIONBIO, &mode);
#else
   int flags = fcntl(conn_socket, F_GETFL, 0);
   fcntl(conn_socket, F_SETFL, (flags | O_NONBLOCK));
#endif


   {
	   int flag = 1;

	   ::setsockopt(
			   conn_socket,
			   IPPROTO_TCP,
			   TCP_NODELAY,
			   (char *)&flag,
			   sizeof(int));

   }

   ITransport *transport = factory->mkSocketTransport(pid, conn_socket);
   IEndpoint *endpoint = factory->mkJsonRpcEndpoint(
		   transport,
		   	  services);

    fprintf(stdout, "connected\n");

    return endpoint;
}

std::string PyTblinkHdlLauncher::last_error() {

}

std::string PyTblinkHdlLauncher::find_python() {
	// Search through the path to find Python
	const char *sp = getenv("PATH");
	std::string python;

	while (sp && *sp) {
		const char *np = strchr(sp, PS);
		char *path;

		if (np) {
			path = (char *)alloca((np-sp)+64);
			memcpy(path, sp, (np-sp));
			path[np-sp] = 0;
			sp = np+1;
		} else {
			path = (char *)alloca(strlen(sp)+64);
			strcpy(path, sp);
			sp = 0;
		}

#ifdef _WIN32
		const char *python_exe[] = {
				"pypy.exe",
				"python3.exe",
				"python.exe",
				0
		};
#else
		const char *python_exe[] = {
				"pypy",
				"python3",
				"python",
				0
		};
#endif
		const char **py_exe_p = python_exe;
		uint32_t init_len = strlen(path);
		while (*py_exe_p) {
			struct stat stat_b;

			path[init_len] = 0;

			strcat(path, "/");
			strcat(path, *py_exe_p);

			fprintf(stdout, "Test: \"%s\"\n", path);

			if (stat(path, &stat_b) == 0 && S_ISREG(stat_b.st_mode)) {
				python = path;
				break;
			}
			py_exe_p++;
		}

		if (python != "") {
			break;
		}
	}

	return python;
}

static PyTblinkHdlLauncher prvLauncher;

extern "C" ILaunch *tblink_rpc_hdl_launcher() {
	return &prvLauncher;
}
