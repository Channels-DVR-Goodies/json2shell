//
// Created by paul on 4/14/22.
//

#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <cjson/cJSON.h>

#include "json2shell.h"

int startFile(const char * path)
{
    (void)path;
    return 0;
}

int emitObject( cJSON * json )
{
    return 0;
}

int finishFile(const char * path)
{
    (void)path;
    return 0;
}
