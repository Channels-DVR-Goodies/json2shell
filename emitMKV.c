//
// Created by paul on 5/19/21.
//

#define _GNU_SOURCE

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
    keyDescription               = 0xac8ee9d559e498e3L,
    keyEpisode                   = 0x07f6e5b84001b72eL,
    keyGenres                    = 0xe26af37b81f5894bL,
    keyIsMovie                   = 0x07f6e5c9f2073d5dL,
    keyIsNews                    = 0xe26af37bc427264aL,
    keyIsSports                  = 0x567898ebb63bfe7eL,
    keyMetadataGenerator         = 0xf9db832b9eb9d75fL,
    keyOriginalBroadcastDateTime = 0x7f8eea1049ded158L,
    keySeriesPremiereDate        = 0xad8a17d96390cc45L,
    keyRecordedDateTime          = 0x464052353f406e08L,
    keySeason                    = 0xe26af37c0ba41858L,
    keySubTitle                  = 0x56789adabe140e4bL,
    keyTitle                     = 0xdb97530eca85ae6bL
};

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

int emitObject( cJSON * json )
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
