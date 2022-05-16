//
// Created by paul on 5/15/21.
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


int processTree( cJSON * json )
{
    int result = 0;

    if ( !cJSON_IsObject(json) )
    {
        fprintf( stderr, "Internal error: expected object at JSON root.\n");
        result = -1;
    }
    else
    {
        result = emitObject( json->child );
    }

    return result;
}

int processArg( char * arg )
{
    int result = 0;

    int fd = open( arg, O_RDONLY );
    if ( fd < 0 )
    {
        result = errno;
        fprintf( stderr, "Unable to open \'%s\' (%d: %s)\n", arg, errno, strerror(errno));
    }
    else
    {
        struct stat fileInfo;
        if ( fstat( fd, &fileInfo ) < 0 )
        {
            result = errno;
            fprintf( stderr, "Unable to get information about \'%s\' (%d: %s)\n", arg, errno, strerror(errno));
        }
        else
        {
            size_t bufferSize = fileInfo.st_size;
            char * buffer = calloc( 1, fileInfo.st_size + 1 );
            if ( buffer == NULL )
            {
                fprintf( stderr, "out of memory trying to read \'%s\' (%d: %s)\n", arg, errno, strerror(errno));
            }
            else
            {
                if ( read( fd, buffer, bufferSize ) < 0 )
                {
                    result = errno;
                    fprintf( stderr, "unable to access \'%s\' (%d: %s)\n", arg, errno, strerror(errno));
                }
                else
                {
#if 0
                    fwrite( buffer, bufferSize, 1, stderr );
                    fputc( '\n', stderr );
#endif
                    cJSON * json = cJSON_Parse( buffer );
                    if ( json == NULL )
                    {
                        fprintf( stderr, "failed to parse JSON file \'%s\'", arg );
                    }
                    else
                    {
                        startFile(arg);
                        processTree( json );
                        finishFile(arg);
#if 0
                        char * jsonAsString = cJSON_Print( json );
                        fprintf( stdout, "%s\n", jsonAsString );
                        free( jsonAsString );
#endif
                        cJSON_Delete( json );
                    }
                }
            }
        }
    }

    return result;
}

int main(int argc, char * argv[] )
{
    int result = 0;

    if ( argc < 2 )
    {
        fprintf( stderr, "Error: please provide at least one JSON file to process.\n");
        result = -1;
    }
    else
    {
#if 0
        for ( int i = 1; i < argc && result == 0; i++ )
        {
            if ( argv[i][0] == '-' && argv[i][2] == '\0' )
            {
                switch (argv[i][1])
                {
                case 'm':
                    /* export an xml tags file for mkvmerge */
                    global.appMode = modeMKVTags;
                    break;

                case 's':
                    /* export equivalent shell syntax to define shell variables */
                    global.appMode = modeShell;
                    break;

                case '-':
                    /* skip the remaining args */
                    i = argc;
                    break;

                default:
                    fprintf( stderr, "Error: don't understand option \'%s\'\n", argv[i] );
                    break;
                }
            }
        }
#endif
        for ( int i = 1; i < argc && result == 0; i++ )
        {
#if 0
            if ( argv[i][0] != '-' && argv[i][2] != '\0' )
#endif
            {
                result = processArg( argv[i] );
            }
        }
    }

    return result;
}