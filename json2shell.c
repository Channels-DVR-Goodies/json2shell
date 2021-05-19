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
#include <sys/types.h>
#include <stdint.h>
#include <limits.h>

#include <cjson/cJSON.h>
#include <ctype.h>
#include <math.h>
#include <stdbool.h>
#include <time.h>

typedef enum {
     modeUnknown = 0,
     modeShell,
     modeMKVTags
} tAppMode;

typedef struct {
    char *     description;
    char *     seriesName;
    char *     episodeName;
    struct tm  firstBroadcast;
    struct tm  seriesPremiere;
    struct tm  recorded;
    char *     generator;
    char **    genres;
    int        genreCount;
    int        season;
    int        episode;
    bool       isMovie;
    bool       isNews;
    bool       isSports;
} tTags;

typedef uint64_t tHash;

enum keyHash {
    keyDescription          = 0xac8ee9d559e498e3L,
    keyEpisode              = 0x07f6e5b84001b72eL,
    keyGenres               = 0xe26af37b81f5894bL,
    keyIsMovie              = 0x07f6e5c9f2073d5dL,
    keyIsNews               = 0xe26af37bc427264aL,
    keyIsSports             = 0x567898ebb63bfe7eL,
    keyMetadataGenerator    = 0xf9db832b9eb9d75fL,
    keyOriginalBroadcastDateTime = 0x7f8eea1049ded158L,
    keySeriesPremiereDate   = 0xad8a17d96390cc45L,
    keyRecordedDateTime     = 0x464052353f406e08L,
    keySeason               = 0xe26af37c0ba41858L,
    keySubTitle             = 0x56789adabe140e4bL,
    keyTitle                = 0xdb97530eca85ae6bL
};

struct {
    tAppMode    appMode;
} global;

tHash hashString( const char * string )
{
    tHash hash = 0x123456789abcdef;

    for ( const char * p = string; *p != '\0'; p++ )
    {
        hash = (hash * 43) ^ tolower(*p);
    }

    return hash;
}

const char * mkvPreamble  = "<Tags>\n <Tag>\n";
const char * mkvPostamble = " </Tag>\n</Tags>\n";

typedef enum {
    mkvCollection   = 70,
    mkvSeason       = 60,
    mkvSequel       = 60,
    mkvVolume       = 60,
    mkvMovie        = 50,
    mkvEpisode      = 50,
    mkvConcert      = 50,
    mkvPart         = 40,
    mkvSession      = 40,
    mkvChapter      = 30,
    mkvScene        = 20,
    mkvShot         = 10
} mkvTarget;

int emitTarget( mkvTarget target )
{
    const char * format = "  <Targets>\n   <TargetTypeValue>%d</TargetTypeValue>\n  </Targets>\n";
    printf( format, target );
}

int emitSimpleTag( const char * key, const char * value )
{
    const char * format = "  <Simple>\n   <Name>%s</Name>\n   <String>%s</String>\n  </Simple>\n";

    if ( key != NULL && value != NULL )
    {
        printf( format, key, value );
    }
}

int emitSimpleTagI( const char * key, int value )
{
    const char * format = "  <Simple>\n   <Name>%s</Name>\n   <String>%d</String>\n  </Simple>\n";
    printf( format, key, value );
}

int emitMKVtags( cJSON * json )
{
    int    result = 0;
    int    i;
    tTags  tags;
    char   temp[80];

    memset( &tags, 0, sizeof(tags) );

    for ( cJSON * j = json; j != NULL; j = j->next )
    {
        // fprintf( stderr, "key%s = 0x%16lxL,\n", j->string, hashString( j->string ) );

        switch ( hashString( j->string ) )
        {
        case keyDescription:
            tags.description = j->valuestring;
            break;

        case keyTitle:
            tags.seriesName = j->valuestring;
            break;

        case keySubTitle:
            tags.episodeName = j->valuestring;
            break;

        case keySeason:
            tags.season = (int) round( j->valuedouble );
            break;

        case keyEpisode:
            tags.episode = (int) round( j->valuedouble );
            break;

        case keyGenres:
            tags.genreCount = 0;
            for ( cJSON * a = j->child; a != NULL; a = a->next )
            {
                ++tags.genreCount;
            }
            tags.genres = calloc( tags.genreCount, sizeof(char *) );
            i = 0;
            for ( cJSON * a = j->child; a != NULL; a = a->next )
            {
                tags.genres[i++] = a->valuestring;
            }
            break;

        case keyIsMovie:
            tags.isMovie = (j->valueint != 0);
            break;

        case keyIsNews:
            tags.isNews = (j->valueint != 0);
            break;

        case keyIsSports:
            tags.isSports = (j->valueint != 0);
            break;

        case keyMetadataGenerator:
            tags.generator = j->valuestring;
            break;

        case keyOriginalBroadcastDateTime:
            strptime( j->valuestring, "%Y-%m-%dT%T%Z", &tags.firstBroadcast );
            break;

        case keySeriesPremiereDate:
            strptime( j->valuestring, "%Y-%m-%dT%T%Z", &tags.seriesPremiere );
            break;

        case keyRecordedDateTime:
            strptime( j->valuestring, "%Y-%m-%dT%T%Z", &tags.recorded );
            break;

        default:
            fprintf( stderr, "Unknown Key: \"%s\"\n", j->string );
            break;
        }
    }

    printf("%s", mkvPreamble );

    if (tags.isMovie)
    {
        emitTarget( mkvMovie );
        emitSimpleTag( "TITLE", tags.seriesName );
    }
    else
    {
        emitTarget( mkvSeason );
        emitSimpleTag( "TITLE", tags.seriesName );
        emitSimpleTagI( "PART_NUMBER", tags.season );
        strftime( temp, sizeof(temp), "%Y-%m-%d", &tags.seriesPremiere );
        emitSimpleTag( "DATE_RELEASED", temp );
        emitTarget( mkvEpisode );
        emitSimpleTag( "TITLE", tags.episodeName );
        emitSimpleTagI( "PART_NUMBER", tags.episode );
    }
    strftime( temp, sizeof(temp), "%Y-%m-%d", &tags.firstBroadcast );
    emitSimpleTag( "DATE_RELEASED", temp );
    emitSimpleTag( "DESCRIPTION", tags.description );

    for ( i = 0; i < tags.genreCount; ++i )
    {
        emitSimpleTag( "GENRE", tags.genres[i] );
    }
    strftime( temp, sizeof(temp), "%Y-%m-%d %H:%M", &tags.recorded );
    emitSimpleTag( "DATE_RECORDED", temp );

    emitSimpleTag( "ENCODER", tags.generator );
    printf("%s", mkvPostamble );

    return result;
}

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

int emitShellObject(cJSON * json )
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
        switch (global.appMode)
        {
        case modeShell:
            result = emitShellObject( json->child );
            break;

        case modeMKVTags:
            result = emitMKVtags( json->child);
            break;
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
        global.appMode = modeShell;

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
        for ( int i = 1; i < argc && result == 0; i++ )
        {
            if ( argv[i][0] != '-' && argv[i][2] != '\0' )
            {
                result = processArg( argv[i] );
            }
        }
    }

    return result;
}