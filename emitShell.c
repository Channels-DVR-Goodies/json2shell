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


int emitShellArray(cJSON * j )
{
    printf( "( ");
    for ( cJSON * p = j; p != NULL; p = p->next )
    {
        printf("\"%s\" ", p->valuestring );
    }
    printf( ")\n");
    return 0;
}

int emitObject( cJSON * json )
{
    int result = 0;

    for ( cJSON * j = json; j != NULL; j = j->next )
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
            result = emitShellArray( j->child );
        }
        else if ( cJSON_IsObject( j ))
        {
            fprintf( stderr, "internal error: nested objects not supported.\n");
            result = -1;
        }
    }

    return result;
}