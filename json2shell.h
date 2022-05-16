//
// Created by paul on 5/19/21.
//

#ifndef JSON2SHELL_JSON2SHELL_H
#define JSON2SHELL_JSON2SHELL_H

int startFile(const char * path);
int emitObject( cJSON * json );
int finishFile(const char * path);


#endif //JSON2SHELL_JSON2SHELL_H
