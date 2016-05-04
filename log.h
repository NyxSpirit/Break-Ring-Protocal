#pragma once
#include <fcntl.h>
#include <stdio.h>
#include "testenv.h"
struct test_logger
{
	int logFiles[DEV_NUMBER];	
	int errorFile;
	int linkFile;
};

struct test_logger logger;


int initLogger();
int logInfo(struct sw_dev* dev, char* info);
int logLink(struct rrpp_link* link, char* info);
int logError(char* error);
int destroyLogger();
