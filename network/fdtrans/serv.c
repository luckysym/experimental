#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <sys/uio.h>

/**
 *  Author: shiyimin@yeah.net    
 *  Date:   3/22/2013
 *
 *	Global Variables:
 *		int client_pipe_fd        与子进程通讯的fd，用于向子进程发送远程socket句柄
 *
 * 	Functions:
 * 		create_inet_server()      创建tcp侦听服务，父进程调用
 * 		create_pipe_server()      创建本地管道侦听服务，父进程调用
 * 		pipe_connect()            子进程连接父进程的管道
 * 		inet_client_connected()   父进程收到远程tcp连接的处理函数，收到连接后将句柄发送到子进程
 * 		pipe_client_connected()   父进程收到子进程的管道连接处理函数，函数中将管道句柄保存为全局变量
 * 		wait_for_connect()        父进程等待socket连接，包括等待子进程管道连接，或者远程tcp连接，
 * 		                          收到连接后根据socket类型分别调用inet_client_connected()
 * 		                          或pipe_client_connected()
 * 		recv_fd()                 子进程用于接收父进程发来的远程socket句柄
 *
 *		child_main()              子进程主函数
 *
 *  程序测试执行过程：
 *  	1. 父进程main函数启动后立即fork一个子进程，然后创建管道监听
 *  	2. 子进程启动后，等待5秒钟后（保证父进程管道创建完毕），连接父进程管道
 *  	3. 父进程收到子进程的管道连接后，保存管道通讯句柄，然后创建tcp侦听（端口30098）
 *  	4. 子进程连接父进程完毕后，等待接收父进程的管道消息
 *
 *  	4. 远程启动一个telnet连接到30098端口。
 *  	5. 父进程收到连接后，首先向远程发送"Hello Client"消息
 *  	6. 然后父进程将tcp连接句柄通过管道发送给子进程（发送过程中句柄被复制）
 *  	7. 发送完毕后，父进程关闭tcp连接句柄
 *  	8. 子进程收到tcp连接句柄后，向远程telnet发送"This is child process speaking"消息
 *  	   然后准备读取该tcp连接句柄。
 *  	9. 远程telnet端输入并发送一条文本消息，子进程在收到消息后会将其打印出（以[child]为前缀）
 *      10. 子进程打印出消息后，关闭tcp连接句柄，telnet端连接同时关闭。
 */

int client_pipe_fd = 0;

int create_inet_server(uint16_t port)
{
	int fd = socket(PF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
	if ( fd < 0 ) {
		printf("failed to create socket, err=%d\n", errno);
		return fd;
	}

	struct sockaddr_in laddr;
	laddr.sin_family = AF_INET;
	laddr.sin_port = htons(port);
	laddr.sin_addr.s_addr = htonl(INADDR_ANY);

	int ret = bind(fd, (struct sockaddr*)&laddr, sizeof(laddr));
	if ( ret < 0 ) {
		printf("failed to bind local port %d, err=%d\n", port, errno);
		return ret;
	}

	ret = listen(fd, 10);
	if ( ret < 0 ) {
		printf("failed to listen, err=%d", errno);
		return ret;
	}

	return fd;
}

int create_pipe_server(const char * path)
{
	int fd = socket(PF_UNIX, SOCK_STREAM , 0);
	if ( fd < 0 ) {
		printf("failed to create pipe, err=%d\n", errno);
		return fd;
	}

	struct sockaddr_un laddr;
	memset(&laddr, 0, sizeof(laddr));
	laddr.sun_family = AF_LOCAL;
	strcpy(laddr.sun_path, path);
	//laddr.sun_len = sizeof(laddr.sun_length) + sizeof(laddr.sun_family) + strlen(laddr.sun_path)  + 1;

	int ret = bind(fd, (struct sockaddr*)&laddr, sizeof(laddr));
	if ( ret < 0 ) {
		printf("failed to bind local path %s, err=%d\n", path, errno);
		return ret;
	}

	ret = listen(fd, 10);
	if ( ret < 0 ) {
		printf("failed to listen pipe, err=%d", errno);
		return ret;
	}

	return fd;
}

int pipe_connect(const char * local, const char * remote)
{
	struct sockaddr_un laddr;
	struct sockaddr_un raddr;

	laddr.sun_family = AF_LOCAL;
	raddr.sun_family = AF_LOCAL;

	strcpy(laddr.sun_path, local);
	strcpy(raddr.sun_path, remote);

	int llen = strlen(laddr.sun_path) + sizeof(laddr.sun_family) + 1;
	int rlen = strlen(raddr.sun_path) + sizeof(raddr.sun_family) + 1;

	unlink(local);
	int fd = socket(PF_UNIX, SOCK_STREAM, 0);
	if (fd < 0) {
		printf("create client pipe socket fail, err=%d\n", errno);
		return fd;
	}
	int ret = bind(fd, (struct sockaddr *)&laddr, llen);
	if ( ret < 0) {
		printf("bind local pipe path fail - %s, %d\n", local, errno);
		return ret;
	}
	
	ret = connect(fd, (struct sockaddr *)&raddr, rlen );
	if (ret < 0 ) {
		printf("connect to remote pipe fail, errno=%d\n", errno);
		return ret;
	}

	return fd;
	
}

int inet_client_connected(int sock, const struct sockaddr * raddr, socklen_t size)
{
	printf("Connection comming\n");
	int ret = write(sock, "Hello Client!\n", 15);

	struct cmsghdr *  cmptr = 0;
	struct msghdr msg;
	struct iovec  iov[1];
	char   buf[2];

	iov[0].iov_base = buf;
	iov[0].iov_len  = 2;
	msg.msg_iov = iov;
	msg.msg_iovlen = 1;
	msg.msg_name = NULL;
	msg.msg_namelen = 0;

	cmptr = (struct cmsghdr *)malloc(sizeof(struct cmsghdr) + sizeof(int));
	cmptr->cmsg_level = SOL_SOCKET;
	cmptr->cmsg_type  = SCM_RIGHTS;
	cmptr->cmsg_len   = sizeof(struct cmsghdr) + sizeof(int);
	*(int *)CMSG_DATA(cmptr) = sock;
	msg.msg_control = cmptr;
	msg.msg_controllen = sizeof(struct cmsghdr) + sizeof(int);
	
	buf[1] = 0; buf[0] = 1;

	ret = sendmsg(client_pipe_fd, &msg, 0);
	if ( ret != 2) {
		printf("send socket fd failed, ret=%d, err=%d\n", ret, errno);
	}

	free(cmptr);
	close(sock);
	return 0;
}

int pipe_client_connected(int sock, const struct sockaddr * addr, socklen_t size)
{
	const struct sockaddr_un * paddr = (const struct sockaddr_un *)addr;
	printf("client pipe connected fd=%d\n", sock);
	client_pipe_fd = sock;
	return -1;
}

#define INET_SOCK 0
#define PIPE_SOCK 1

int wait_for_connect(int sock, int type)
{
	fd_set rds;
	
	int ret = 0;
	struct timeval times;

	struct sockaddr_in riaddr;
	struct sockaddr_un ruaddr;
	socklen_t rsize[] = { sizeof(riaddr), sizeof(ruaddr) };
	struct sockaddr  * paddrs[] = { (struct sockaddr*)&riaddr, (struct sockaddr*)&ruaddr};

	while ( !ret ) {
		times.tv_sec = 1;
		times.tv_usec = 0;
		FD_ZERO(&rds);
		FD_SET(sock, &rds);

		ret = select(sock + 1, &rds, 0, 0, &times);
		if ( ret < 0 ) {
			printf("select fail. err=%d\n", errno);
			break;
		} else if ( ret == 0) {
			continue;
		} else {
			ret = accept(sock, paddrs[type], &rsize[type]);
			if ( ret < 0 ) {
				break;
			} else {
				if ( type == INET_SOCK) {
					ret = inet_client_connected(ret, paddrs[INET_SOCK], rsize[INET_SOCK]);
				} else if ( type == PIPE_SOCK ) {
					ret = pipe_client_connected(ret, paddrs[PIPE_SOCK], rsize[PIPE_SOCK]);
				}
				if ( ret < 0 ) break;
			}
		}
		ret = 0;
	} // while

	return 0;
}

int recv_fd(int pipefd)
{
	char buf[2];
	int newfd = -1;
	struct iovec   iov[1];
	struct msghdr  msg;
	struct cmsghdr * cmptr = (struct cmsghdr *)malloc(sizeof(struct cmsghdr) + sizeof(int));
	
	iov[0].iov_base = buf;
	iov[0].iov_len  = sizeof(buf);
	msg.msg_iov     = iov;
	msg.msg_iovlen  = 1;
	msg.msg_name    = NULL;
	msg.msg_namelen = 0;
	msg.msg_control = cmptr;
	msg.msg_controllen = sizeof(struct cmsghdr) + sizeof(int);

	int ret = recvmsg(pipefd, &msg, 0);
	printf("[child] recvmsg returned, ret=%d\n", ret);
	if ( ret < 0 ) {
		printf("recvmsg failed, err=%d\n", errno);
	} else {
		newfd = *(int*)CMSG_DATA(cmptr);
		printf("[child] recv the socket fd: %d\n", newfd);
	}
	free(cmptr);
	return newfd;
}

int child_main()
{
	printf("[child] child process running, wait 5 secs for pipe server creation\n");
	sleep(5);
	int fd = pipe_connect("/tmp/pipe_local", "/tmp/fd_trans");
	printf("[child] connect to remote pipe fd = %d\n", fd);
	
	int newfd = recv_fd(fd);
	int ret = write(newfd, "[child] This is child process speaking!\n", 41);
	if ( ret < 0 ) {
		printf("[child] write message to remote fail: ret=%d, err=%d\n", ret, errno);
	}

	char buf[256];
	memset(buf, 0, sizeof(buf));
	ret = read(newfd, buf, 256);
	if ( ret < 0) {
		printf("[child] failed to recv remote message: \n");
	} else {
		buf[ret] = 0;
		printf("[child] remote message arrived: %s\n", buf);
	}
	printf("[child] child exiting\n");
	close(fd);
	close(newfd);
	return 0;
}

int main(int argc, char *argv)
{
	if ( fork() == 0 ) {
		exit(child_main());
	}
	unlink("/tmp/fd_trans");
	int pipesock = create_pipe_server("/tmp/fd_trans");
	
	if (pipesock >= 0) {
		printf("pipe server created, now wait for child connecting\n");
		wait_for_connect(pipesock, PIPE_SOCK);
	}
	int inetsock = create_inet_server(30098);
	if ( inetsock >= 0 ) wait_for_connect(inetsock, INET_SOCK);

	close(client_pipe_fd);
	close(pipesock);
	close(inetsock);
	return 0;
}

