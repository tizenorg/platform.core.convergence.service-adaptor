CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(sal-api)

SET(CMAKE_SKIP_BUILD_RPATH TRUE)

IF("${CMAKE_BUILD_TYPE}" STREQUAL "")
        SET(CMAKE_BUILD_TYPE "Debug")
ENDIF("${CMAKE_BUILD_TYPE}" STREQUAL "")

MESSAGE("")
MESSAGE(">>> current directory: ${CMAKE_CURRENT_SOURCE_DIR}")
MESSAGE(">>> Build type: ${CMAKE_BUILD_TYPE}")

SET(VISIBILITY "-DEXPORT_API=\"__attribute__((visibility(\\\"default\\\")))\"")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${VISIBILITY} -fvisibility=hidden")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,--gc-sections")

ADD_DEFINITIONS("-DSERVICE_ADAPTOR_DEBUGGING")
ADD_DEFINITIONS("-D_SERVICE_ADAPTOR_IPC_CLIENT")

##########################################################
# Define Service Adaptor CLIENT
##########################################################

SET(ADAPTOR-CLIENT-LIB "service-adaptor-client")
SET(ADAPTOR-CLIENT-SRCS
        ${CMAKE_SOURCE_DIR}/api/client/sal_service_adaptor.c
#        ${CMAKE_SOURCE_DIR}/api/client/sal_service_task.c
#        ${CMAKE_SOURCE_DIR}/api/client/sal_service_auth.c
#        ${CMAKE_SOURCE_DIR}/api/client/sal_service_storage.c
)

INCLUDE_DIRECTORIES(
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/api/client
        ${CMAKE_SOURCE_DIR}/common/base
        ${CMAKE_SOURCE_DIR}/common/ipc-client
#        ${CMAKE_SOURCE_DIR}/adaptor/auth-adaptor
#        ${CMAKE_SOURCE_DIR}/adaptor/contact-adaptor
#        ${CMAKE_SOURCE_DIR}/adaptor/storage-adaptor
)

INCLUDE(FindPkgConfig)
pkg_check_modules(adaptor_client_pkgs REQUIRED dlog glib-2.0 capi-appfw-application bundle)

FOREACH(flag ${adaptor_client_pkgs_CFLAGS})
        SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS} -std=gnu99")
SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--as-needed")

ADD_LIBRARY(${ADAPTOR-CLIENT-LIB} SHARED ${ADAPTOR-CLIENT-SRCS})

TARGET_LINK_LIBRARIES(${ADAPTOR-CLIENT-LIB} ${adaptor_client_pkgs_LDFLAGS} sal-common-base sal-common-client)
SET_TARGET_PROPERTIES(${ADAPTOR-CLIENT-LIB} PROPERTIES SOVERSION ${VERSION_MAJOR})
SET_TARGET_PROPERTIES(${ADAPTOR-CLIENT-LIB} PROPERTIES VERSION ${VERSION})

INSTALL(TARGETS ${ADAPTOR-CLIENT-LIB} DESTINATION ${LIB_INSTALL_DIR} COMPONENT RuntimeLibraries)

SET(SAL-ADAPTOR-CLIENT-HEADERS
        ${CMAKE_SOURCE_DIR}/include/service_adaptor_client.h
        ${CMAKE_SOURCE_DIR}/include/service_adaptor_client_plugin.h
#        ${CMAKE_SOURCE_DIR}/include/service_adaptor_client_storage.h
        ${CMAKE_SOURCE_DIR}/include/service_adaptor_type.h
#        ${CMAKE_SOURCE_DIR}/include/service_adaptor_errors.h
#        ${CMAKE_SOURCE_DIR}/include/service_adaptor_internal.h
#        ${CMAKE_SOURCE_DIR}/client/sal_service_adaptor.h
#        ${CMAKE_SOURCE_DIR}/client/sal_service_task.h
#        ${CMAKE_SOURCE_DIR}/client/sal_service_auth.h
#        ${CMAKE_SOURCE_DIR}/client/sal_service_storage.h
#        ${CMAKE_SOURCE_DIR}/common/base/sal_ipc.h
#        ${CMAKE_SOURCE_DIR}/common/ipc-client/sal_ipc_client.h
#        ${CMAKE_SOURCE_DIR}/common/ipc-client/sal_ipc_client_core.h
#        ${CMAKE_SOURCE_DIR}/common/ipc-client/sal_ipc_client_auth.h
#        ${CMAKE_SOURCE_DIR}/common/ipc-client/sal_ipc_client_storage.h
)

INSTALL(FILES ${SAL-ADAPTOR-CLIENT-HEADERS} DESTINATION include/service-adaptor)

##########################################################
# Define Service Provider CLIENT
##########################################################

