cmake_minimum_required(VERSION 3.8)
project(ixg)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GSTLIBS REQUIRED
        gobject-2.0
        glib-2.0
        gstreamer-check-1.0
        )

#pkg_check_modules(JSONCPP jsoncpp)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(Threads)



# add extra include directories
# gst-config.h moved to /usr/lib/x86_64-linux-gnu/gstreamer-1.0/include
include_directories(
        /usr/lib/x86_64-linux-gnu/glib-2.0/include
        /usr/include/glib-2.0
        /usr/include/gstreamer-1.0
        /usr/local/include/gstreamer-1.0
        #/usr/local/include/json
        #/usr/include/jsoncpp
       )

link_directories(${GSTLIBS_LIBRARY_DIRS})

add_library(ixg SHARED  gst.cpp)

#set(SOURCE_FILES ixg.cpp)


#add_executable(ixgEncoder ${SOURCE_FILES})

#target_link_libraries(ixgEncoder ixg)

target_include_directories(ixg PUBLIC ${CMAKE_CURRENT_SOURCE_DIR/lib})