CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(sal-server)

SET(CMAKE_SKIP_BUILD_RPATH TRUE)

IF("${CMAKE_BUILD_TYPE}" STREQUAL "")
        SET(CMAKE_BUILD_TYPE "Debug")
ENDIF("${CMAKE_BUILD_TYPE}" STREQUAL "")

MESSAGE("")
MESSAGE(">>> current directory: ${CMAKE_CURRENT_SOURCE_DIR}")
MESSAGE(">>> Build type: ${CMAKE_BUILD_TYPE}")

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${VISIBILITY} -fvisibility=hidden")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,--gc-sections -fPIE")


##########################################################
# Define Execute File
##########################################################

SET(MAIN-EXE "service-adaptor")
SET(MAIN-SRCS
        ${CMAKE_SOURCE_DIR}/server/sal_auth.c
        ${CMAKE_SOURCE_DIR}/server/sal_contact.c
        ${CMAKE_SOURCE_DIR}/server/sal_storage.c
        ${CMAKE_SOURCE_DIR}/server/sal_observer.c
        ${CMAKE_SOURCE_DIR}/server/sal_main.c
)

INCLUDE_DIRECTORIES(
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/common
        ${CMAKE_SOURCE_DIR}/common/ipc
        ${CMAKE_SOURCE_DIR}/server
        ${CMAKE_SOURCE_DIR}/adaptor/auth-adaptor
        ${CMAKE_SOURCE_DIR}/adaptor/contact-adaptor
        ${CMAKE_SOURCE_DIR}/adaptor/storage-adaptor
)

INCLUDE(FindPkgConfig)
pkg_check_modules(main_pkgs REQUIRED dlog glib-2.0 capi-base-common capi-appfw-app-manager capi-appfw-package-manager libsmack service-discovery service-federation)

FOREACH(flag ${main_pkgs_CFLAGS})
        SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS}")
SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--as-needed -pie")

ADD_DEFINITIONS("-DSERVICE_ADAPTOR_DEBUGGING")
ADD_DEFINITIONS("-D_SERVICE_ADAPTOR_IPC_SERVER")

ADD_EXECUTABLE(${MAIN-EXE} ${MAIN-SRCS})
TARGET_LINK_LIBRARIES(${MAIN-EXE} ${main_pkgs_LDFLAGS} sal-common auth-adaptor contact-adaptor storage-adaptor)
INSTALL(TARGETS ${MAIN-EXE} DESTINATION bin)
