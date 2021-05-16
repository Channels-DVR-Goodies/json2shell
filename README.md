# json2shell

Parses the provided JSON file, and outputs it as equivalent shell commands to set
shell variables.

It is not very sophisticated, it doesn't need to be for my purposes. I would have
just used `jq`, but it has a few shortcomings that meant I couldn't make it work
for parsing MCE Buddy files emitted by Channels DVR.

### Typical usage:

    eval $( json2shell <json filename> )

## Use with Channels DVR

Channels DVR can optionally emit JSON files alongside recordings, in the style used
by MCE Buddy. Unfortunately MCE Buddy is exclusive to Windows. The information is
still valuable so I wanted to take advantage of that extra info when post-processing
Channels DVR recordings.

This little tool gets the information in those simple JSON files into shell
variables so I can use them easily in my scripts.

### Example

Suppose I have a file `Courageous Love (2017) 2021-05-13-2200.json` that Channels DVR
created which has the contents:
```json
{"Description":"Alex has to choose between saving his family's company or following his heart.", 
 "Episode":0,"Genres":["Drama","Romance"],"IsMovie":true,"IsNews":false,"IsSports":false,
 "MetadataGenerator":"Channels DVR","OriginalBroadcastDateTime":"2017-03-21T00:00:00Z",
 "RecordedDateTime":"2021-05-13T22:00:00Z","Season":0,"SubTitle":"",
 "Title":"Courageous Love (2017)"}
```

running `json2shell` on that file produces:

```bash
Description="Alex has to choose between saving his family's company or following his heart."
Episode=0
Genres=( "Drama" "Romance" )
IsMovie=1
IsNews=0
IsSports=0
MetadataGenerator="Channels DVR"
OriginalBroadcastDateTime="2017-03-21T00:00:00Z"
RecordedDateTime="2021-05-13T22:00:00Z"
Season=0
SubTitle=""
Title="Courageous Love (2017)"
```
Noote that the Genres array in the JSON file has been converted to the `bash` shell
syntax for array variables.

By using `eval` on the output of json2shell, the metadata from the JSON file
will be available as equivalent shell variables in shell scripts. This can be
useful to determine where to link the recording in a Plex media heirarchy,
for example.

For example, my spouse enjoys a particular genre of movie, and so in my scripts, 
those movies are moved directly to her personal Movie folder, not the main one
that the family shares.

## Acknoledgements
This executable depends on the [cjson library](https://github.com/DaveGamble/cJSON) to parse JSON into something C 
can digest. Thank you, Dave Gamble.