cmake_minimum_required(VERSION 3.0)

project(sound-system)

find_path(
    GSTREAMER_INCLUDE gst/gst.h 
    HINTS /usr/include/gstreamer-1.0)
find_path(
    GLIB_INCLUDE glib.h 
    HINTS /usr/include/glib-2.0)
find_path(
    GLIB_CONFIG_INCLUDE 
    glibconfig.h 
    HINTS 
    /usr/lib/x86_64-linux-gnu/glib-2.0/include
    /usr/lib/arm-linux-gnueabihf/glib-2.0/include)


add_executable(server main.c server.c config.c logger.c)

target_include_directories(
    server 
    PRIVATE 
    ${GLIB_INCLUDE} 
    ${GLIB_CONFIG_INCLUDE} 
    ${GSTREAMER_INCLUDE})

target_link_libraries(server cjson glib-2.0 gstrtspserver-1.0 gobject-2.0 pthread)