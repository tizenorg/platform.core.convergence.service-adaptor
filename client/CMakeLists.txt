CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(sal-client)

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

##########################################################
# Define Service Adaptor CLIENT
##########################################################

SET(CLIENT-LIB "sal-client")
SET(CLIENT-SRCS
        ${CMAKE_SOURCE_DIR}/client/sal_core_api.c
)

INCLUDE_DIRECTORIES(
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/common
        ${CMAKE_SOURCE_DIR}/common/ipc
        ${CMAKE_SOURCE_DIR}/client
        ${CMAKE_SOURCE_DIR}/adaptor/auth-adaptor
        ${CMAKE_SOURCE_DIR}/adaptor/contact-adaptor
        ${CMAKE_SOURCE_DIR}/adaptor/storage-adaptor
)

INCLUDE(FindPkgConfig)
pkg_check_modules(client_pkgs REQUIRED dlog glib-2.0)

FOREACH(flag ${client_pkgs_CFLAGS})
        SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS}")

SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--as-needed")

ADD_LIBRARY(${CLIENT-LIB} SHARED ${CLIENT-SRCS})

TARGET_LINK_LIBRARIES(${CLIENT-LIB} ${client_pkgs_LDFLAGS} sal-common)
SET_TARGET_PROPERTIES(${CLIENT-LIB} PROPERTIES SOVERSION ${VERSION_MAJOR})
SET_TARGET_PROPERTIES(${CLIENT-LIB} PROPERTIES VERSION ${VERSION})

INSTALL(TARGETS ${CLIENT-LIB} DESTINATION lib COMPONENT RuntimeLibraries)

SET(SAL-CLIENT-HEADERS
        ${CMAKE_SOURCE_DIR}/include/service_adaptor.h
        ${CMAKE_SOURCE_DIR}/include/service_adaptor_types.h
        ${CMAKE_SOURCE_DIR}/include/service_adaptor_errors.h
        ${CMAKE_SOURCE_DIR}/include/service_adaptor_internal.h
)

INSTALL(FILES ${SAL-CLIENT-HEADERS} DESTINATION include/service-adaptor)
