//
// Created by paul on 5/19/21.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <limits.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>
#include <time.h>
#include <time.h>

#include <cjson/cJSON.h>
#include <memory.h>

#include "json2shell.h"


static int emitQuotedString( const char * str )
{
    const char * p;
    char * q;
    int quotedCount = strlen(str) + 2;
    for ( p = str; *p != '\0'; ++p )
    {
        switch (*p)
        {
        case '"':
            quotedCount++;
            break;
        }
    }

    char * quotedString = malloc(quotedCount);
    if ( quotedString != NULL )
    {
        p = str;
        q = quotedString;
        *q++ = '"';
        while (*p != '\0' )
        {
            if (*p == '"')
            {
                *q++ = '\\';
            }
            *q++ = *p++;
        }
        *q++ = '"';
        *q = '\0';

        printf( "%s", quotedString );

        free( quotedString );
    }

}

static int emitShellArray(cJSON * j )
{
    printf( "( ");
    for ( cJSON * p = j; p != NULL; p = p->next )
    {
        emitQuotedString( p->valuestring );
        fputc( ' ', stdout );
    }
    printf( ")");
    return 0;
}


int startFile(const char * path)
{
    (void)path;
    return 0;
}

int emitObject( cJSON * json )
{
    int result = 0;

    for ( cJSON * j = json; j != NULL; j = j->next )
    {
        if ( cJSON_IsString( j ) )
        {
            printf( "%s=", j->string);
            emitQuotedString( j->valuestring );
        }
        else if ( cJSON_IsNumber( j ))
        {
            printf( "%s=%g", j->string, j->valuedouble);
        }
        else if ( cJSON_IsBool( j ))
        {
            printf( "%s=%d", j->string, j->valueint);
        }
        else if ( cJSON_IsArray( j ))
        {
            printf( "%s=", j->string);
            result = emitShellArray( j->child );
        }
        else if ( cJSON_IsObject( j ))
        {
            fprintf( stderr, "internal error: nested objects not supported.\n");
            result = -1;
        }
        fputc('\n',stdout);
    }

    return result;
}

int finishFile(const char * path)
{
    (void)path;
    return 0;
}
