CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(sal-common)

SET(CMAKE_SKIP_BUILD_RPATH TRUE)

IF("${CMAKE_BUILD_TYPE}" STREQUAL "")
        SET(CMAKE_BUILD_TYPE "Debug")
ENDIF("${CMAKE_BUILD_TYPE}" STREQUAL "")

MESSAGE("")
MESSAGE(">>> current directory: ${CMAKE_CURRENT_SOURCE_DIR}")
MESSAGE(">>> Build type: ${CMAKE_BUILD_TYPE}")

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${VISIBILITY} -fvisibility=hidden")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,--gc-sections")

ADD_DEFINITIONS("-DSERVICE_ADAPTOR_DEBUGGING")

##########################################################
# Define common base
##########################################################

SET(PLUGIN-CONFIG-LIB "plugin-config")
SET(PLUGIN-CONFIG-SRCS
        ${CMAKE_SOURCE_DIR}/common/plugin_config/plugin_message.c
)

INCLUDE_DIRECTORIES(
        ${CMAKE_SOURCE_DIR}/common/plugin_config
)

INCLUDE(FindPkgConfig)
pkg_check_modules(plugin_config_pkgs REQUIRED dlog glib-2.0 gio-2.0 gobject-2.0 json-glib-1.0)

FOREACH(flag ${plugin_config_pkgs_CFLAGS})
        SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS} -std=gnu99")

SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--as-needed")

ADD_LIBRARY(${PLUGIN-CONFIG-LIB} SHARED ${PLUGIN-CONFIG-SRCS})
TARGET_LINK_LIBRARIES(${PLUGIN-CONFIG-LIB} ${plugin_config_pkgs_LDFLAGS})
SET_TARGET_PROPERTIES(${PLUGIN-CONFIG-LIB} PROPERTIES SOVERSION ${VERSION_MAJOR})
SET_TARGET_PROPERTIES(${PLUGIN-CONFIG-LIB} PROPERTIES VERSION ${VERSION})

INSTALL(TARGETS ${PLUGIN-CONFIG-LIB} DESTINATION ${LIB_INSTALL_DIR} COMPONENT RuntimeLibraries)

