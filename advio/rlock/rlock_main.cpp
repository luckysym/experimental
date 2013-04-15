#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#define FILE_PATH "records.txt"



int main(int argc, char *argv[])
{
	// open a file first
	int fd = open(FILE_PATH, O_RDWR);
	if ( fd < 0) {
		printf("ERR: failed to open file %s\n", FILE_PATH);
		return 0;
	}

	struct flock  flk;
	flk.l_type  = F_RDLCK;
	flk.l_start = 0;
	flk.l_len   = 0;
	flk.l_whence = SEEK_SET;

	int ret = fcntl(fd, F_SETLK, &flk);
	if ( ret < 0 ) {
		printf("ERR: failed to set lock. %s\n", strerror(errno));
		close(fd);
		return 0;
	}

	pid_t pid = fork();
	if ( pid == 0) {
		ret = fcntl(fd, F_SETLK, &flk);
		if ( ret < 0) {
			printf("CHILD: set read lock fail, %s\n", strerror(errno));
		} else {
			printf("CHILD: set read lock ok\n");
		}
		flk.l_type = F_UNLCK;
		fcntl(fd, F_SETLK, &flk);
		flk.l_type = F_WRLCK;
		ret = fcntl(fd, F_SETLK, &flk);
		if ( ret < 0) {
			printf("CHILD: set write lock fail, %s\n", strerror(errno));
		} else {
			printf("CHILD: set write lock ok\n");
		}
	} else {
		sleep(5);  // parent wait for child to set the lock, if no wait, 
		           //   parent will exit before child set the lock
	}	
	close(fd);
}


