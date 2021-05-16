//
// Created by paul on 5/15/21.
//

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>

#include <cjson/cJSON.h>

int processArray( cJSON * j )
{
    printf( "( ");
    for ( cJSON * p = j; p != NULL; p = p->next )
    {
        printf("\"%s\" ", p->valuestring );
    }
    printf( ")\n");
    return 0;
}

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
        for ( cJSON * j = json->child; j != NULL; j = j->next )
        {
            if ( cJSON_IsString( j ) )
            {
                printf( "%s=\"%s\"\n", j->string, j->valuestring);
            }
            else if ( cJSON_IsNumber( j ))
            {
                printf( "%s=%g\n", j->string, j->valuedouble);
            }
            else if ( cJSON_IsBool( j ))
            {
                printf( "%s=%d\n", j->string, j->valueint);
            }
            else if ( cJSON_IsArray( j ))
            {
                printf( "%s=", j->string);
                result = processArray( j->child );
            }
            else if ( cJSON_IsObject( j ))
            {
                fprintf( stderr, "internal error: nested objects not supported.\n");
                result = -1;
            }
        }
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
                        processTree( json );
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
        for ( int i = 1; i < argc && result == 0; i++ )
        {
            result = processArg( argv[i] );
        }
    }

    return result;
}