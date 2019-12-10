#  Debug          : w/  debug symbols, w/o optimization
#  Release        : w/o debug symbols, w/  optimization
#  RelWithDebInfo : w/  debug symbols, w/  optimization
#  MinSizeRel     : w/o debug symbols, w/  optimization, stripped binaries
cmake_minimum_required( VERSION 3.0 )

include( "${CMAKE_SOURCE_DIR}/Purify/Loader.cmake" )

create_build( GLOBAL_DEFINE )
