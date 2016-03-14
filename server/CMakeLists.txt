CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(sal-server)

SET(CMAKE_SKIP_BUILD_RPATH TRUE)

IF("${CMAKE_BUILD_TYPE}" STREQUAL "")
        SET(CMAKE_BUILD_TYPE "Debug")
ENDIF("${CMAKE_BUILD_TYPE}" STREQUAL "")

MESSAGE("")
MESSAGE(">>> current directory: ${CMAKE_CURRENT_SOURCE_DIR}")
MESSAGE(">>> Build type: ${CMAKE_BUILD_TYPE}")

#SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${VISIBILITY} -fvisibility=hidden")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,--gc-sections -fPIE")

ADD_DEFINITIONS("-DAUTH_PLUGIN_PATH=\"${PROVIDER_PATH}/auth\"")
ADD_DEFINITIONS("-DSTORAGE_PLUGIN_PATH=\"${PROVIDER_PATH}/storage\"")
ADD_DEFINITIONS("-DCONTACT_PLUGIN_PATH=\"${PROVIDER_PATH}/contact\"")
ADD_DEFINITIONS("-DMESSAGE_PLUGIN_PATH=\"${PROVIDER_PATH}/message\"")
ADD_DEFINITIONS("-DPUSH_PLUGIN_PATH=\"${PROVIDER_PATH}/push\"")
ADD_DEFINITIONS("-DSHOP_PLUGIN_PATH=\"${PROVIDER_PATH}/shop\"")

##########################################################
# Define Execute File
##########################################################

SET(MAIN-EXE "service-adaptor-server")
FILE(GLOB MAIN-SRCS
        ${CMAKE_SOURCE_DIR}/server/src/*.c
        ${CMAKE_SOURCE_DIR}/server/src/dbus/*.c
        ${CMAKE_SOURCE_DIR}/server/src/util/*.c
)

INCLUDE_DIRECTORIES(
        ${CMAKE_SOURCE_DIR}/common/plugin_config

        ${CMAKE_SOURCE_DIR}/adaptor/auth-adaptor
        ${CMAKE_SOURCE_DIR}/adaptor/contact-adaptor
        ${CMAKE_SOURCE_DIR}/adaptor/storage-adaptor
        ${CMAKE_SOURCE_DIR}/adaptor/message-adaptor
        ${CMAKE_SOURCE_DIR}/adaptor/push-adaptor
        ${CMAKE_SOURCE_DIR}/adaptor/shop-adaptor

        ${CMAKE_SOURCE_DIR}/server/inc
        ${CMAKE_SOURCE_DIR}/server/inc/dbus
        ${CMAKE_SOURCE_DIR}/server/inc/util
)

INCLUDE(FindPkgConfig)
pkg_check_modules(main_pkgs REQUIRED
        glib-2.0
        gobject-2.0
        gio-2.0
        gthread-2.0
        dlog
        bundle
        capi-base-common
        capi-appfw-application
        capi-appfw-app-manager
        capi-appfw-package-manager
        cynara-client
        cynara-session
        cynara-creds-gdbus
        libtzplatform-config
)

FOREACH(flag ${main_pkgs_CFLAGS})
        SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS} -std=gnu99")
SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--as-needed -pie")

ADD_DEFINITIONS("-DSERVICE_ADAPTOR_DEBUGGING")
ADD_DEFINITIONS("-D_SERVICE_ADAPTOR_IPC_SERVER")

ADD_EXECUTABLE(${MAIN-EXE} ${MAIN-SRCS})
TARGET_LINK_LIBRARIES(${MAIN-EXE} ${main_pkgs_LDFLAGS} plugin-config auth-adaptor contact-adaptor storage-adaptor message-adaptor push-adaptor shop-adaptor)
INSTALL(TARGETS ${MAIN-EXE} DESTINATION bin)

SET(SAL-SERVER-HEADERS
        ${CMAKE_SOURCE_DIR}/server/inc/dbus/dbus-server.h
        ${CMAKE_SOURCE_DIR}/server/inc/service-adaptor.h
)

INSTALL(FILES ${SAL-SERVER-HEADERS} DESTINATION include/service-adaptor)
