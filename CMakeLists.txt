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

set(LOGGER_INCLUDE src/include/logger)

set(A2DP_INCLUDE src/include/a2dp)


option(CLIENT "build sound system client")
option(SERVER "build sound system server")

add_library(logger SHARED src/logger/logger.c)
target_include_directories(logger PRIVATE ${LOGGER_INCLUDE})

if(${SERVER})
    add_executable(server src/server/main.c src/server/config.c src/server/config.h src/server/server.c src/server/server.h)
    target_include_directories(
        server 
        PRIVATE 
        ${LOGGER_INCLUDE}
        ${GLIB_INCLUDE} 
        ${GLIB_CONFIG_INCLUDE} 
        ${GSTREAMER_INCLUDE})
    target_link_libraries(server cjson logger glib-2.0 gstrtspserver-1.0 gstreamer-1.0 gobject-2.0 pthread)
endif()

if(${CLIENT})
    add_executable(client src/client/main.c)
    target_include_directories(
        client
        PRIVATE 
        ${LOGGER_INCLUDE}
        ${GLIB_INCLUDE} 
        ${GLIB_CONFIG_INCLUDE} 
        ${GSTREAMER_INCLUDE}
        "/usr/include/dbus-1.0"
        "/usr/lib/arm-linux-gnueabihf/dbus-1.0/include")
    target_link_libraries(client dbus-1 cjson logger glib-2.0 gobject-2.0 gio-2.0 pthread)
endif()