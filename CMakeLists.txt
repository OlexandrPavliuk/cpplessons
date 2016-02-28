cmake_minimum_required(VERSION 2.6)
project(final)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(SOURCE_EXE httpserver.cpp main.cpp)
set(HEADER httpserver.h)

set( Boost_USE_STATIC_LIBS OFF )
find_package( Boost REQUIRED COMPONENTS system filesystem )
include_directories( ${Boost_INCLUDE_DIRS} )

find_package (Threads)

add_executable(final ${SOURCE_EXE})
target_link_libraries(final ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT}) 

install(TARGETS final RUNTIME DESTINATION bin)
