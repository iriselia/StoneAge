SET( DEFINE
)
SET( INCLUDE
)
SET( LINK
3rdParty/mysql/lib/libmysql.lib
)
create_project(STATIC "${DEFINE}" "${INCLUDE}" "${LINK}")
