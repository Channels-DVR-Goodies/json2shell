cmake_minimum_required( VERSION 3.10 )

project( json2shell )

add_executable( json2shell json2shell.c)

target_link_libraries( json2shell m cjson )

install( TARGETS json2shell
         RUNTIME DESTINATION /usr/bin)