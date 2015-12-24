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

SET(COMMON-BASE-LIB "sal-common-base")
SET(COMMON-BASE-SRCS
        ${CMAKE_SOURCE_DIR}/common/base/sal_ipc.c
)

INCLUDE_DIRECTORIES(
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/common/base
)

INCLUDE(FindPkgConfig)
pkg_check_modules(common_base_pkgs REQUIRED dlog glib-2.0 gio-2.0)

FOREACH(flag ${common_base_pkgs_CFLAGS})
        SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS} -std=gnu99")
#message(">>>[${COMMON-BASE-LIB}] extra_cflags ${EXTRA_CFLAGS}")
#message(">>>[${COMMON-BASE-LIB}] common_base_pkgs_cflags ${common_base_pkgs_CFLAGS}")
#message(">>>[${COMMON-BASE-LIB}] result ${CMAKE_C_FLAGS}")

SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--as-needed")

ADD_LIBRARY(${COMMON-BASE-LIB} SHARED ${COMMON-BASE-SRCS})
TARGET_LINK_LIBRARIES(${COMMON-BASE-LIB} ${common_base_pkgs_LDFLAGS})
SET_TARGET_PROPERTIES(${COMMON-BASE-LIB} PROPERTIES SOVERSION ${VERSION_MAJOR})
SET_TARGET_PROPERTIES(${COMMON-BASE-LIB} PROPERTIES VERSION ${VERSION})

INSTALL(TARGETS ${COMMON-BASE-LIB} DESTINATION ${LIB_INSTALL_DIR} COMPONENT RuntimeLibraries)

SET(SAL-COMMON_BASE-HEADERS
        ${CMAKE_SOURCE_DIR}/common/base/sal_log.h
        ${CMAKE_SOURCE_DIR}/common/base/sal_ipc.h
        ${CMAKE_SOURCE_DIR}/common/base/sal_types.h
)

INSTALL(FILES ${SAL-COMMON-BASE-HEADERS} DESTINATION include/service-adaptor)

##########################################################
# Define common server
##########################################################

SET(COMMON-SERVER-LIB "sal-common-server")
SET(COMMON-SERVER-SRCS
        ${CMAKE_SOURCE_DIR}/common/ipc-server/sal_ipc_server.c
        ${CMAKE_SOURCE_DIR}/common/ipc-server/sal_ipc_server_core.c
        ${CMAKE_SOURCE_DIR}/common/ipc-server/sal_ipc_server_auth.c
        ${CMAKE_SOURCE_DIR}/common/ipc-server/sal_ipc_server_storage.c
)

INCLUDE_DIRECTORIES(
#        ${CMAKE_SOURCE_DIR}/include
#        ${CMAKE_SOURCE_DIR}/adaptor/auth-adaptor
#        ${CMAKE_SOURCE_DIR}/adaptor/contact-adaptor
#        ${CMAKE_SOURCE_DIR}/adaptor/storage-adaptor
#        ${CMAKE_SOURCE_DIR}/adaptor/resource-adaptor
#        ${CMAKE_SOURCE_DIR}/common/base
        ${CMAKE_SOURCE_DIR}/common/ipc-server
#        ${CMAKE_SOURCE_DIR}/server
)

INCLUDE(FindPkgConfig)
pkg_check_modules(common_server_pkgs REQUIRED dlog glib-2.0 gio-2.0)

FOREACH(flag ${common_server_pkgs_CFLAGS})
        SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS} -std=gnu99")
#message(">>>[${COMMON-SERVER-LIB}] extra_cflags ${EXTRA_CFLAGS}")
#message(">>>[${COMMON-SERVER-LIB}] common_server_pkgs_cflags ${common_server_pkgs_CFLAGS}")
#message(">>>[${COMMON-SERVER-LIB}] result ${CMAKE_C_FLAGS}")

SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--as-needed")

ADD_LIBRARY(${COMMON-SERVER-LIB} SHARED ${COMMON-SERVER-SRCS})
TARGET_LINK_LIBRARIES(${COMMON-SERVER-LIB} ${common_server_pkgs_LDFLAGS} sal-common-base)
SET_TARGET_PROPERTIES(${COMMON-SERVER-LIB} PROPERTIES SOVERSION ${VERSION_MAJOR})
SET_TARGET_PROPERTIES(${COMMON-SERVER-LIB} PROPERTIES VERSION ${VERSION})

INSTALL(TARGETS ${COMMON-SERVER-LIB} DESTINATION ${LIB_INSTALL_DIR} COMPONENT RuntimeLibraries)

##########################################################
# Define common client
##########################################################

SET(COMMON-CLIENT-LIB "sal-common-client")
SET(COMMON-CLIENT-SRCS
        ${CMAKE_SOURCE_DIR}/common/ipc-client/sal_ipc_client.c
        ${CMAKE_SOURCE_DIR}/common/ipc-client/sal_ipc_client_core.c
        ${CMAKE_SOURCE_DIR}/common/ipc-client/sal_ipc_client_auth.c
        ${CMAKE_SOURCE_DIR}/common/ipc-client/sal_ipc_client_storage.c
)

INCLUDE_DIRECTORIES(
#        ${CMAKE_SOURCE_DIR}/include
#        ${CMAKE_SOURCE_DIR}/common/base
        ${CMAKE_SOURCE_DIR}/common/ipc-client
#        ${CMAKE_SOURCE_DIR}/client
)

INCLUDE(FindPkgConfig)
pkg_check_modules(common_client_pkgs REQUIRED dlog glib-2.0 gio-2.0)

FOREACH(flag ${common_client_pkgs_CFLAGS})
        SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS} -std=gnu99")
#message(">>>[${COMMON-CLIENT-LIB}] extra_cflags ${EXTRA_CFLAGS}")
#message(">>>[${COMMON-CLIENT-LIB}] common_client_pkgs_cflags ${common_client_pkgs_CFLAGS}")
#message(">>>[${COMMON-CLIENT-LIB}] result ${CMAKE_C_FLAGS}")

SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--as-needed")

ADD_LIBRARY(${COMMON-CLIENT-LIB} SHARED ${COMMON-CLIENT-SRCS})
TARGET_LINK_LIBRARIES(${COMMON-CLIENT-LIB} ${common_client_pkgs_LDFLAGS} sal-common-base)
SET_TARGET_PROPERTIES(${COMMON-CLIENT-LIB} PROPERTIES SOVERSION ${VERSION_MAJOR})
SET_TARGET_PROPERTIES(${COMMON-CLIENT-LIB} PROPERTIES VERSION ${VERSION})

INSTALL(TARGETS ${COMMON-CLIENT-LIB} DESTINATION ${LIB_INSTALL_DIR} COMPONENT RuntimeLibraries)

##########################################################
# Define common provider
##########################################################

SET(COMMON-PROVIDER-LIB "sal-common-provider")
SET(COMMON-PROVIDER-SRCS
        ${CMAKE_SOURCE_DIR}/common/ipc-provider/sal_ipc_provider.c
        ${CMAKE_SOURCE_DIR}/common/ipc-provider/sal_ipc_provider_base.c
        ${CMAKE_SOURCE_DIR}/common/ipc-provider/sal_ipc_provider_storage.c
)

INCLUDE_DIRECTORIES(
        ${CMAKE_SOURCE_DIR}/common/ipc-provider
)

INCLUDE(FindPkgConfig)
pkg_check_modules(common_provider_pkgs REQUIRED dlog glib-2.0 gio-2.0 capi-appfw-application)

FOREACH(flag ${common_provider_pkgs_CFLAGS})
        SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS} -std=gnu99")

SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--as-needed")

ADD_LIBRARY(${COMMON-PROVIDER-LIB} SHARED ${COMMON-PROVIDER-SRCS})
TARGET_LINK_LIBRARIES(${COMMON-PROVIDER-LIB} ${common_provider_pkgs_LDFLAGS} sal-common-base)
SET_TARGET_PROPERTIES(${COMMON-PROVIDER-LIB} PROPERTIES SOVERSION ${VERSION_MAJOR})
SET_TARGET_PROPERTIES(${COMMON-PROVIDER-LIB} PROPERTIES VERSION ${VERSION})

INSTALL(TARGETS ${COMMON-PROVIDER-LIB} DESTINATION ${LIB_INSTALL_DIR} COMPONENT RuntimeLibraries)


