/** 测试用例：
 *		测试驱动程序 sys_notify() 函数
 *  测试方式：
 * 		./sys_notify &
 * 		echo 1234 > /sys/bus/tes/devices/TesDev@000/value 或者 等待10s
 *  desc:
 *  	使用poll在 /sys/bus/tes/devices/TesDev@000/sleep 属性文件上进入睡眠，10s超时
 *  	往  /sys/bus/tes/devices/TesDev@000/value 写值时，驱动程序调用 sys_notify() 唤醒poll
 *  	sleep 返回的是value的值
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <poll.h>
#include <sys/epoll.h>

//#define USE_EPOLL

#define TES_FILE 	"/sys/bus/tes/devices/TesDev@000/sleep"
#define POLL_CNT	6
#define TIME_OUT	(10*1000)

int main(int argc, char **argv)
{
	int tesFd, pollCnt, ret;
	ssize_t readCnt;
	char value[4] = {0};

	pollCnt = POLL_CNT;
	tesFd = open(TES_FILE, O_RDONLY);
	if (tesFd < 0) {
		printf("open %s failed!\n", TES_FILE);
		return -1;
	}

	/** need read dummy once */
	read(tesFd, value, sizeof(value));

#ifdef USE_EPOLL
	int epollFd;
	struct epoll_event event;
	
	event.events = EPOLLWAKEUP | EPOLLERR;
	
	epollFd = epoll_create(1);
	ret = epoll_ctl(epollFd, EPOLL_CTL_ADD, tesFd, &event);
	if (ret) {
		printf("epoll_ctl failed!\n");
	}
	while (pollCnt--) {
		ret = epoll_wait(epollFd, &event, 1, TIME_OUT);
		if (ret == 0) {
			printf("epoll_wait time out!\n");
		} else if (ret < 0) {
			printf("epoll_wait failed!\n");
		} else {
			lseek(tesFd, 0, SEEK_SET);
			readCnt = read(tesFd, value, sizeof(value));
			printf("now can read! value is %s\n", value);
		}
	}
#else

	struct pollfd fds;

	fds.fd = tesFd;
	fds.events = POLLPRI | POLLERR;

	while (pollCnt--) {
		ret = poll(&fds, 1, TIME_OUT);
		if (ret == 0) {
			printf("poll time out!\n");
		} else if (ret < 0) {
			printf("poll failed!\n");
		} else {
			lseek(tesFd, 0, SEEK_SET);
			readCnt = read(tesFd, value, sizeof(value));
			printf("now can read! value is %s\n", value);
		}
	}

	#if 0
		/** select test failed !!! */

		fd_set readFdSet;
		struct timeval timeOut;

		FD_ZERO(&readFdSet);
		FD_SET(tesFd, &readFdSet);
		timeOut.tv_sec = 10;
		timeOut.tv_usec = 0;
		while (1) {
			ret = select(tesFd + 1, &readFdSet, NULL, NULL, &timeOut);
			if (ret < 0) {
				printf("select return fail! ret:%d\n", ret);
			} else if (ret > 0) {
				if (FD_ISSET(tesFd, &readFdSet)) {
					lseek(tesFd, 0, SEEK_SET);
					readCnt = read(tesFd, value, 4);
					if (readCnt < 0) {
						printf("read %s failed!\n", TES_FILE);
						close(tesFd);
						return -1;
					} else if (readCnt == 0) {
						printf("read zero!\n");
					} else {
						printf("read %s\n", value);
					}
				}
			} else {
				printf("timeout\n");
			}
		}
	#endif
#endif

	close(tesFd);

	return 0;
}
