# Install script for directory: /Users/akram/Developer/pangolin-hdr/pangolin

# Set the install prefix
IF(NOT DEFINED CMAKE_INSTALL_PREFIX)
  SET(CMAKE_INSTALL_PREFIX "/usr/local")
ENDIF(NOT DEFINED CMAKE_INSTALL_PREFIX)
STRING(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
IF(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  IF(BUILD_TYPE)
    STRING(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  ELSE(BUILD_TYPE)
    SET(CMAKE_INSTALL_CONFIG_NAME "Release")
  ENDIF(BUILD_TYPE)
  MESSAGE(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
ENDIF(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)

# Set the component getting installed.
IF(NOT CMAKE_INSTALL_COMPONENT)
  IF(COMPONENT)
    MESSAGE(STATUS "Install component: \"${COMPONENT}\"")
    SET(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  ELSE(COMPONENT)
    SET(CMAKE_INSTALL_COMPONENT)
  ENDIF(COMPONENT)
ENDIF(NOT CMAKE_INSTALL_COMPONENT)

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  list(APPEND CPACK_ABSOLUTE_DESTINATION_FILES
   "/usr/local/include/pangolin/config.h;/usr/local/include/pangolin/cg.h;/usr/local/include/pangolin/display.h;/usr/local/include/pangolin/display_internal.h;/usr/local/include/pangolin/gl.h;/usr/local/include/pangolin/glcuda.h;/usr/local/include/pangolin/pangolin.h;/usr/local/include/pangolin/platform.h;/usr/local/include/pangolin/plotter.h;/usr/local/include/pangolin/simple_math.h;/usr/local/include/pangolin/vars.h;/usr/local/include/pangolin/vars_internal.h;/usr/local/include/pangolin/video.h;/usr/local/include/pangolin/firewire.h;/usr/local/include/pangolin/widgets.h")
FILE(INSTALL DESTINATION "/usr/local/include/pangolin" TYPE FILE FILES
    "/Users/akram/Developer/pangolin-hdr/build/pangolin/config.h"
    "/Users/akram/Developer/pangolin-hdr/pangolin/cg.h"
    "/Users/akram/Developer/pangolin-hdr/pangolin/display.h"
    "/Users/akram/Developer/pangolin-hdr/pangolin/display_internal.h"
    "/Users/akram/Developer/pangolin-hdr/pangolin/gl.h"
    "/Users/akram/Developer/pangolin-hdr/pangolin/glcuda.h"
    "/Users/akram/Developer/pangolin-hdr/pangolin/pangolin.h"
    "/Users/akram/Developer/pangolin-hdr/pangolin/platform.h"
    "/Users/akram/Developer/pangolin-hdr/pangolin/plotter.h"
    "/Users/akram/Developer/pangolin-hdr/pangolin/simple_math.h"
    "/Users/akram/Developer/pangolin-hdr/pangolin/vars.h"
    "/Users/akram/Developer/pangolin-hdr/pangolin/vars_internal.h"
    "/Users/akram/Developer/pangolin-hdr/pangolin/video.h"
    "/Users/akram/Developer/pangolin-hdr/pangolin/video/firewire.h"
    "/Users/akram/Developer/pangolin-hdr/pangolin/widgets.h"
    )
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  IF("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    list(APPEND CPACK_ABSOLUTE_DESTINATION_FILES
     "/usr/local/lib/libpangolin.a")
FILE(INSTALL DESTINATION "/usr/local/lib" TYPE STATIC_LIBRARY FILES "/Users/akram/Developer/pangolin-hdr/build/pangolin/Debug/libpangolin.a")
    IF(EXISTS "$ENV{DESTDIR}/usr/local/lib/libpangolin.a" AND
       NOT IS_SYMLINK "$ENV{DESTDIR}/usr/local/lib/libpangolin.a")
      EXECUTE_PROCESS(COMMAND "/usr/bin/ranlib" "$ENV{DESTDIR}/usr/local/lib/libpangolin.a")
    ENDIF()
  ELSEIF("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    list(APPEND CPACK_ABSOLUTE_DESTINATION_FILES
     "/usr/local/lib/libpangolin.a")
FILE(INSTALL DESTINATION "/usr/local/lib" TYPE STATIC_LIBRARY FILES "/Users/akram/Developer/pangolin-hdr/build/pangolin/Release/libpangolin.a")
    IF(EXISTS "$ENV{DESTDIR}/usr/local/lib/libpangolin.a" AND
       NOT IS_SYMLINK "$ENV{DESTDIR}/usr/local/lib/libpangolin.a")
      EXECUTE_PROCESS(COMMAND "/usr/bin/ranlib" "$ENV{DESTDIR}/usr/local/lib/libpangolin.a")
    ENDIF()
  ELSEIF("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
    list(APPEND CPACK_ABSOLUTE_DESTINATION_FILES
     "/usr/local/lib/libpangolin.a")
FILE(INSTALL DESTINATION "/usr/local/lib" TYPE STATIC_LIBRARY FILES "/Users/akram/Developer/pangolin-hdr/build/pangolin/MinSizeRel/libpangolin.a")
    IF(EXISTS "$ENV{DESTDIR}/usr/local/lib/libpangolin.a" AND
       NOT IS_SYMLINK "$ENV{DESTDIR}/usr/local/lib/libpangolin.a")
      EXECUTE_PROCESS(COMMAND "/usr/bin/ranlib" "$ENV{DESTDIR}/usr/local/lib/libpangolin.a")
    ENDIF()
  ELSEIF("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    list(APPEND CPACK_ABSOLUTE_DESTINATION_FILES
     "/usr/local/lib/libpangolin.a")
FILE(INSTALL DESTINATION "/usr/local/lib" TYPE STATIC_LIBRARY FILES "/Users/akram/Developer/pangolin-hdr/build/pangolin/RelWithDebInfo/libpangolin.a")
    IF(EXISTS "$ENV{DESTDIR}/usr/local/lib/libpangolin.a" AND
       NOT IS_SYMLINK "$ENV{DESTDIR}/usr/local/lib/libpangolin.a")
      EXECUTE_PROCESS(COMMAND "/usr/bin/ranlib" "$ENV{DESTDIR}/usr/local/lib/libpangolin.a")
    ENDIF()
  ENDIF()
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

