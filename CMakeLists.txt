cmake_minimum_required(VERSION 3.8)
project(ixgencoder)

find_package(PkgConfig REQUIRED)

pkg_check_modules(GSTLIBS REQUIRED
    gobject-2.0
    glib-2.0
    gstreamer-check-1.0
)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(Threads REQUIRED)

pkg_check_modules(JSONCPP REQUIRED jsoncpp)

# Add extra include directories
include_directories(
    ${GSTLIBS_INCLUDE_DIRS}
    ${JSONCPP_INCLUDE_DIRS}
    /usr/lib/x86_64-linux-gnu/glib-2.0/include
    /usr/include/glib-2.0
    /usr/include/gstreamer-1.0
    /usr/local/include/gstreamer-1.0
    /usr/local/include/json
    /usr/include/jsoncpp
)

link_directories(${GSTLIBS_LIBRARY_DIRS})

add_library(ixg SHARED src/gst.cpp src/gstUtility.cpp)

set(SOURCE_FILES ixg.cpp jsonParser.cpp)

add_executable(ixgencoder ${SOURCE_FILES})

target_link_libraries(ixg PUBLIC ${GSTLIBS_LIBRARIES} Threads::Threads)
target_link_libraries(ixgencoder PUBLIC ixg ${JSONCPP_LIBRARIES} ${GSTLIBS_LIBRARIES} Threads::Threads)
