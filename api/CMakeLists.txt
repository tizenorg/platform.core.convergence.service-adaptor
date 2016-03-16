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
#SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${VISIBILITY} -fvisibility=hidden")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,--gc-sections")

ADD_DEFINITIONS("-DSERVICE_ADAPTOR_DEBUGGING")
ADD_DEFINITIONS("-D_SERVICE_ADAPTOR_IPC_CLIENT")

#ADD_DEFINITIONS("-D__DEBUG_GLIB_ERROR")


##########################################################
# Define Service Adaptor CLIENT
##########################################################

SET(ADAPTOR-CLIENT-LIB "service-adaptor-client")
FILE(GLOB ADAPTOR-CLIENT-SRCS
        ${CMAKE_SOURCE_DIR}/api/client/src/service_adaptor_client.c
	${CMAKE_SOURCE_DIR}/api/client/src/service_adaptor_client_storage.c
        ${CMAKE_SOURCE_DIR}/api/client/src/dbus/dbus_client.c
        ${CMAKE_SOURCE_DIR}/api/client/src/dbus/dbus_client_layer.c
        ${CMAKE_SOURCE_DIR}/api/client/src/dbus/dbus_client_storage.c
        ${CMAKE_SOURCE_DIR}/api/client/src/util/service_adaptor_client_util.c
)

INCLUDE_DIRECTORIES(
        ${CMAKE_SOURCE_DIR}/server/inc
        ${CMAKE_SOURCE_DIR}/server/inc/dbus

        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/api/client/include
        ${CMAKE_SOURCE_DIR}/api/client/include/private
        ${CMAKE_SOURCE_DIR}/api/client/include/dbus
        ${CMAKE_SOURCE_DIR}/api/client/include/util
)

INCLUDE(FindPkgConfig)
pkg_check_modules(adaptor_client_pkgs REQUIRED dlog glib-2.0 capi-appfw-application bundle)

FOREACH(flag ${adaptor_client_pkgs_CFLAGS})
        SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS} -std=gnu99")
SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--as-needed")

ADD_LIBRARY(${ADAPTOR-CLIENT-LIB} SHARED ${ADAPTOR-CLIENT-SRCS})

TARGET_LINK_LIBRARIES(${ADAPTOR-CLIENT-LIB} ${adaptor_client_pkgs_LDFLAGS})
SET_TARGET_PROPERTIES(${ADAPTOR-CLIENT-LIB} PROPERTIES SOVERSION ${VERSION_MAJOR})
SET_TARGET_PROPERTIES(${ADAPTOR-CLIENT-LIB} PROPERTIES VERSION ${VERSION})

INSTALL(TARGETS ${ADAPTOR-CLIENT-LIB} DESTINATION ${LIB_INSTALL_DIR} COMPONENT RuntimeLibraries)

SET(SAL-ADAPTOR-CLIENT-HEADERS
        ${CMAKE_SOURCE_DIR}/include/service_adaptor_client.h
        ${CMAKE_SOURCE_DIR}/include/service_adaptor_client_plugin.h
        ${CMAKE_SOURCE_DIR}/include/service_adaptor_client_storage.h

        ${CMAKE_SOURCE_DIR}/api/client/include/service_adaptor_client_private.h
        ${CMAKE_SOURCE_DIR}/api/client/include/service_adaptor_client_storage_internal.h
        ${CMAKE_SOURCE_DIR}/api/client/include/service_adaptor_client_type.h

        ${CMAKE_SOURCE_DIR}/api/client/include/private/service-adaptor-client.h
        ${CMAKE_SOURCE_DIR}/api/client/include/private/service-adaptor-client-auth.h
        ${CMAKE_SOURCE_DIR}/api/client/include/private/service-adaptor-client-contact.h
        ${CMAKE_SOURCE_DIR}/api/client/include/private/service-adaptor-client-message.h
        ${CMAKE_SOURCE_DIR}/api/client/include/private/service-adaptor-client-push.h
        ${CMAKE_SOURCE_DIR}/api/client/include/private/service-adaptor-client-shop.h
        ${CMAKE_SOURCE_DIR}/api/client/include/private/service-adaptor-client-storage.h
)

INSTALL(FILES ${SAL-ADAPTOR-CLIENT-HEADERS} DESTINATION include/service-adaptor)

