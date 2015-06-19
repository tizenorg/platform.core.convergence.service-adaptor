CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(sal-adaptor)

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
ADD_DEFINITIONS("-D_SERVICE_ADAPTOR_IPC_SERVER")

##########################################################
# Define auth adaptor lib
##########################################################

SET(AUTH-LIB "auth-adaptor")
SET(AUTH-SRCS
        ./auth-adaptor/oauth1_service.c
        ./auth-adaptor/oauth2_service.c
        ./auth-adaptor/auth_adaptor.c
)

INCLUDE_DIRECTORIES(
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/client
        ${CMAKE_SOURCE_DIR}/adaptor/auth-adaptor
)

INCLUDE(FindPkgConfig)
pkg_check_modules(auth_pkgs REQUIRED dlog glib-2.0 capi-appfw-application)

set(EXTRA_CFLAGS "")
FOREACH(flag ${auth_pkgs_CFLAGS})
        SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS} -std=gnu99")

#message(">>> extra_cflags ${EXTRA_CFLAGS}")
#message(">>> result ${CMAKE_C_FLAGS}")
#message(">>> ld flag ${device_pkgs_LDFLAGS}")

SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--as-needed")

ADD_LIBRARY(${AUTH-LIB} SHARED ${AUTH-SRCS})
TARGET_LINK_LIBRARIES(${AUTH-LIB} ${auth_pkgs_LDFLAGS})
SET_TARGET_PROPERTIES(${AUTH-LIB} PROPERTIES SOVERSION ${VERSION_MAJOR})
SET_TARGET_PROPERTIES(${AUTH-LIB} PROPERTIES VERSION ${VERSION})

INSTALL(TARGETS ${AUTH-LIB} DESTINATION lib COMPONENT RuntimeLibraries)


##########################################################
# Define contact adaptor lib
##########################################################

SET(CONTACT-LIB "contact-adaptor")
SET(CONTACT-SRCS
        ./contact-adaptor/contact_adaptor.c
)

INCLUDE_DIRECTORIES(
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/adaptor/contact-adaptor
)

INCLUDE(FindPkgConfig)
pkg_check_modules(contact_pkgs REQUIRED dlog glib-2.0 capi-appfw-application)

set(EXTRA_CFLAGS "")
FOREACH(flag ${contact_pkgs_CFLAGS})
        SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS} -std=gnu99")

#message(">>> extra_cflags ${EXTRA_CFLAGS}")
#message(">>> result ${CMAKE_C_FLAGS}")
#message(">>> ld flag ${device_pkgs_LDFLAGS}")

SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--as-needed")

ADD_LIBRARY(${CONTACT-LIB} SHARED ${CONTACT-SRCS})
TARGET_LINK_LIBRARIES(${CONTACT-LIB} ${contact_pkgs_LDFLAGS})
SET_TARGET_PROPERTIES(${CONTACT-LIB} PROPERTIES SOVERSION ${VERSION_MAJOR})
SET_TARGET_PROPERTIES(${CONTACT-LIB} PROPERTIES VERSION ${VERSION})

INSTALL(TARGETS ${CONTACT-LIB} DESTINATION lib COMPONENT RuntimeLibraries)


##########################################################
# Define storage adaptor lib
##########################################################

SET(STORAGE-LIB "storage-adaptor")
SET(STORAGE-SRCS
        ./storage-adaptor/storage_adaptor.c
        ./storage-adaptor/cloud_service.c
)

INCLUDE_DIRECTORIES(
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/adaptor/storage-adaptor
)

INCLUDE(FindPkgConfig)
pkg_check_modules(storage_pkgs REQUIRED dlog glib-2.0 capi-appfw-application)

set(EXTRA_CFLAGS "")
FOREACH(flag ${storage_pkgs_CFLAGS})
        SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS} -std=gnu99")

#message(">>> extra_cflags ${EXTRA_CFLAGS}")
#message(">>> result ${CMAKE_C_FLAGS}")
#message(">>> ld flag ${storage_pkgs_LDFLAGS}")

SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--as-needed")

ADD_LIBRARY(${STORAGE-LIB} SHARED ${STORAGE-SRCS})
TARGET_LINK_LIBRARIES(${STORAGE-LIB} ${storage_pkgs_LDFLAGS})
SET_TARGET_PROPERTIES(${STORAGE-LIB} PROPERTIES SOVERSION ${VERSION_MAJOR})
SET_TARGET_PROPERTIES(${STORAGE-LIB} PROPERTIES VERSION ${VERSION})

INSTALL(TARGETS ${STORAGE-LIB} DESTINATION lib COMPONENT RuntimeLibraries)

##########################################################
# Define resource adaptor lib
##########################################################

SET(RESOURCE-LIB "resource-adaptor")
SET(RESOURCE-SRCS
        ./resource-adaptor/resource_adaptor.c
)

INCLUDE_DIRECTORIES(
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/adaptor/resource-adaptor
)

INCLUDE(FindPkgConfig)
pkg_check_modules(resource_pkgs REQUIRED dlog glib-2.0 capi-appfw-application)

set(EXTRA_CFLAGS "")
FOREACH(flag ${resource_pkgs_CFLAGS})
        SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS} -std=gnu99")

#message(">>> extra_cflags ${EXTRA_CFLAGS}")
#message(">>> result ${CMAKE_C_FLAGS}")
#message(">>> ld flag ${resource_pkgs_LDFLAGS}")

SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--as-needed")

ADD_LIBRARY(${RESOURCE-LIB} SHARED ${RESOURCE-SRCS})
TARGET_LINK_LIBRARIES(${RESOURCE-LIB} ${resource_pkgs_LDFLAGS})
SET_TARGET_PROPERTIES(${RESOURCE-LIB} PROPERTIES SOVERSION ${VERSION_MAJOR})
SET_TARGET_PROPERTIES(${RESOURCE-LIB} PROPERTIES VERSION ${VERSION})

INSTALL(TARGETS ${RESOURCE-LIB} DESTINATION lib COMPONENT RuntimeLibraries)
