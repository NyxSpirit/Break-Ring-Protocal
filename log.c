#include "log.h"
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>


int initLogger()
{
	int i = 0;
	for(i = 0; i < DEV_NUMBER; i++)
	{
		char no[10];

		sprintf(no, "%d", gl_devs[i].rrpp_domains[0].node_id);
		char logpath[100] = "./log/dev";

		logger.logFiles[i] = open(strcat(logpath, no), O_WRONLY|O_CREAT|O_NONBLOCK,S_IRWXU|S_IRWXG|S_IRWXO);
		
	}
	logger.errorFile = open("./log/error", O_WRONLY|O_CREAT|O_APPEND|O_NONBLOCK, S_IRWXU|S_IRWXG|S_IRWXO);
	logger.linkFile = open("./log/link", O_WRONLY|O_CREAT|O_APPEND|O_NONBLOCK, S_IRWXU|S_IRWXG|S_IRWXO);
	return 0;

}

int logInfo(struct sw_dev* dev, char* info)
{
	int i = 0;
	time_t now;
	struct tm* tm_now;
	struct timeval tv;
	gettimeofday(&tv, NULL);
	tm_now = localtime(&tv.tv_sec);
	char buf[150];
	for(i = 0; i < DEV_NUMBER; i++)
	{
		if(&gl_devs[i] == dev) 
		{
			sprintf(buf, "[dev %d][%d:%d:%d:%d]:%s\n", dev->rrpp_domains[0].node_id,  tm_now->tm_hour, tm_now->tm_min, tm_now->tm_sec, (int)tv.tv_usec, info);
			write(logger.logFiles[i], buf, strlen(buf));
		}
	}
	return 0;
}
int logError(char *error)
{
	time_t now;
	struct tm* tm_now;
	struct timeval tv;
	gettimeofday(&tv, NULL);
	tm_now = localtime(&tv.tv_sec);
	char buf[150];
	sprintf(buf, "[%d:%d:%d][%d]:%s\n", tm_now->tm_hour, tm_now->tm_min, tm_now->tm_sec, (int)tv.tv_usec, error);
	write(logger.errorFile, buf, strlen(buf));
	return 0;	
}
int logLink(struct rrpp_link* link, char* info)
{
	time_t now;
	struct tm* tm_now;
	time(&now);
	tm_now = localtime(&now);
	struct timeval tv;
	gettimeofday(&tv, NULL);
	char buf[150];
	sprintf(buf, "[link %d - %d][%d:%d:%d][%d]:%s\n", link->node_id[0], link->node_id[1], tm_now->tm_hour, tm_now->tm_min, tm_now->tm_sec, (int)tv.tv_usec, info);
	write(logger.linkFile, buf, strlen(buf));
	return 0;	
}


int destroyLogger()
{
	int i = 0;
	for(i = 0; i < DEV_NUMBER; i++)
	{
		close(logger.logFiles[i]);
	}
	close(logger.errorFile);
	return 0;
}
