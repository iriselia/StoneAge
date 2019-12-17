SET( DEFINE
_CRT_SECURE_NO_WARNINGS
_SASQL
)
SET( INCLUDE
mysql
)
SET( LINK
mysql
3rdParty/mysql/lib/libmysql.lib
Ws2_32.lib
)
create_project(CONSOLE DEFINE INCLUDE LINK)
add_definitions("/utf-8")
#SET(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} /utf-8")