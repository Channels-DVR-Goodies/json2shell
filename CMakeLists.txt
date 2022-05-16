cmake_minimum_required( VERSION 3.10 )

project( json2shell )

add_executable( json2shell json2shell.c emitShell.c json2shell.h)
target_link_libraries( json2shell m cjson )

add_executable( json2mkv   json2shell.c emitMKV.c json2shell.h)
target_link_libraries( json2mkv m cjson )

add_executable( json2rename   json2shell.c rename.c json2shell.h)
target_link_libraries( json2rename m cjson )

install( TARGETS json2shell json2mkv json2rename
         RUNTIME DESTINATION /usr/bin)