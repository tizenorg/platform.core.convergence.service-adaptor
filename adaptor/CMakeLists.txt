CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(sal-adaptor)

SET(CMAKE_SKIP_BUILD_RPATH TRUE)

IF("${CMAKE_BUILD_TYPE}" STREQUAL "")
        SET(CMAKE_BUILD_TYPE "Debug")
ENDIF("${CMAKE_BUILD_TYPE}" STREQUAL "")

MESSAGE("")
MESSAGE(">>> current directory: ${CMAKE_CURRENT_SOURCE_DIR}")
MESSAGE(">>> Build type: ${CMAKE_BUILD_TYPE}")


#SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${VISIBILITY} -fvisibility=hidden")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,--gc-sections")

ADD_DEFINITIONS("-DSERVICE_ADAPTOR_DEBUGGING")
ADD_DEFINITIONS("-D_SERVICE_ADAPTOR_IPC_SERVER")

ADD_DEFINITIONS("-DDEBUG_ADAPTOR_PARAMS")

##########################################################
# Define auth adaptor lib
##########################################################

SET(AUTH-LIB "auth-adaptor")
SET(AUTH-SRCS
        ${CMAKE_SOURCE_DIR}/adaptor/auth-adaptor/auth-adaptor.c
)

INCLUDE_DIRECTORIES(
        ${CMAKE_SOURCE_DIR}/common/plugin_config
        ${CMAKE_SOURCE_DIR}/adaptor/auth-adaptor/
)

INCLUDE(FindPkgConfig)
pkg_check_modules(auth_pkgs REQUIRED dlog glib-2.0)

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

#INSTALL(FILES ${CMAKE_SOURCE_DIR}/adaptor/auth-adaptor/auth-adaptor.h DESTINATION /usr/include/)
INSTALL(TARGETS ${AUTH-LIB} DESTINATION ${LIB_INSTALL_DIR} COMPONENT RuntimeLibraries)

SET(AUTH-ADAPTOR-HEADERS
        ${CMAKE_SOURCE_DIR}/adaptor/auth-adaptor/auth-adaptor.h
)
INSTALL(FILES ${AUTH-ADAPTOR-HEADERS} DESTINATION include/service-provider)

##########################################################
# Define contact adaptor lib
##########################################################

SET(CONTACT-LIB "contact-adaptor")
SET(CONTACT-SRCS
        ${CMAKE_SOURCE_DIR}/adaptor/contact-adaptor/contact-adaptor.c
)

INCLUDE_DIRECTORIES(
        ${CMAKE_SOURCE_DIR}/adaptor/contact-adaptor
)

INCLUDE(FindPkgConfig)
pkg_check_modules(contact_pkgs REQUIRED dlog glib-2.0)

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

#INSTALL(FILES ${CMAKE_SOURCE_DIR}/adaptor/contact-adaptor/contact-adaptor.h DESTINATION /usr/include/)
INSTALL(TARGETS ${CONTACT-LIB} DESTINATION ${LIB_INSTALL_DIR} COMPONENT RuntimeLibraries)

SET(CONTACT-ADAPTOR-HEADERS
        ${CMAKE_SOURCE_DIR}/adaptor/contact-adaptor/contact-adaptor.h
)
INSTALL(FILES ${CONTACT-ADAPTOR-HEADERS} DESTINATION include/service-provider)

##########################################################
# Define storage adaptor lib
##########################################################

SET(STORAGE-LIB "storage-adaptor")
SET(STORAGE-SRCS
        ${CMAKE_SOURCE_DIR}/adaptor/storage-adaptor/storage-adaptor.c
)

INCLUDE_DIRECTORIES(
        ${CMAKE_SOURCE_DIR}/common/plugin_config
        ${CMAKE_SOURCE_DIR}/adaptor/storage-adaptor
)

INCLUDE(FindPkgConfig)
pkg_check_modules(storage_pkgs REQUIRED dlog glib-2.0)

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

#INSTALL(FILES ${CMAKE_SOURCE_DIR}/adaptor/storage-adaptor/storage-adaptor.h DESTINATION /usr/include/)
INSTALL(TARGETS ${STORAGE-LIB} DESTINATION ${LIB_INSTALL_DIR} COMPONENT RuntimeLibraries)

SET(STORAGE-ADAPTOR-HEADERS
        ${CMAKE_SOURCE_DIR}/adaptor/storage-adaptor/storage-adaptor.h
)
INSTALL(FILES ${STORAGE-ADAPTOR-HEADERS} DESTINATION include/service-provider)

##########################################################
# Define message adaptor lib
##########################################################

SET(MESSAGE-LIB "message-adaptor")
SET(MESSAGE-SRCS
        ${CMAKE_SOURCE_DIR}/adaptor/message-adaptor/message-adaptor.c
)

INCLUDE_DIRECTORIES(
        ${CMAKE_SOURCE_DIR}/adaptor/message-adaptor
)

INCLUDE(FindPkgConfig)
pkg_check_modules(message_pkgs REQUIRED dlog glib-2.0)

set(EXTRA_CFLAGS "")
FOREACH(flag ${message_pkgs_CFLAGS})
        SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS} -std=gnu99")

#message(">>> extra_cflags ${EXTRA_CFLAGS}")
#message(">>> result ${CMAKE_C_FLAGS}")
#message(">>> ld flag ${resource_pkgs_LDFLAGS}")

SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--as-needed")

ADD_LIBRARY(${MESSAGE-LIB} SHARED ${MESSAGE-SRCS})
TARGET_LINK_LIBRARIES(${MESSAGE-LIB} ${message_pkgs_LDFLAGS})
SET_TARGET_PROPERTIES(${MESSAGE-LIB} PROPERTIES SOVERSION ${VERSION_MAJOR})
SET_TARGET_PROPERTIES(${MESSAGE-LIB} PROPERTIES VERSION ${VERSION})

#INSTALL(FILES ${CMAKE_SOURCE_DIR}/adaptor/message-adaptor/message-adaptor.h DESTINATION /usr/include/)
INSTALL(TARGETS ${MESSAGE-LIB} DESTINATION ${LIB_INSTALL_DIR} COMPONENT RuntimeLibraries)

SET(MESSAGE-ADAPTOR-HEADERS
        ${CMAKE_SOURCE_DIR}/adaptor/message-adaptor/message-adaptor.h
)
INSTALL(FILES ${MESSAGE-ADAPTOR-HEADERS} DESTINATION include/service-provider)

##########################################################
# Define push adaptor lib
##########################################################

SET(PUSH-LIB "push-adaptor")
SET(PUSH-SRCS
        ${CMAKE_SOURCE_DIR}/adaptor/push-adaptor/push-adaptor.c
)

INCLUDE_DIRECTORIES(
        ${CMAKE_SOURCE_DIR}/adaptor/push-adaptor
)

INCLUDE(FindPkgConfig)
pkg_check_modules(push_pkgs REQUIRED dlog glib-2.0)

set(EXTRA_CFLAGS "")
FOREACH(flag ${push_pkgs_CFLAGS})
        SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS} -std=gnu99")

#message(">>> extra_cflags ${EXTRA_CFLAGS}")
#message(">>> result ${CMAKE_C_FLAGS}")
#message(">>> ld flag ${resource_pkgs_LDFLAGS}")

SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--as-needed")

ADD_LIBRARY(${PUSH-LIB} SHARED ${PUSH-SRCS})
TARGET_LINK_LIBRARIES(${PUSH-LIB} ${push_pkgs_LDFLAGS})
SET_TARGET_PROPERTIES(${PUSH-LIB} PROPERTIES SOVERSION ${VERSION_MAJOR})
SET_TARGET_PROPERTIES(${PUSH-LIB} PROPERTIES VERSION ${VERSION})

#INSTALL(FILES ${CMAKE_SOURCE_DIR}/adaptor/push-adaptor/push-adaptor.h DESTINATION /usr/include/)
INSTALL(TARGETS ${PUSH-LIB} DESTINATION ${LIB_INSTALL_DIR} COMPONENT RuntimeLibraries)

SET(PUSH-ADAPTOR-HEADERS
        ${CMAKE_SOURCE_DIR}/adaptor/push-adaptor/push-adaptor.h
)
INSTALL(FILES ${PUSH-ADAPTOR-HEADERS} DESTINATION include/service-provider)

##########################################################
# Define shop adaptor lib
##########################################################

SET(SHOP-LIB "shop-adaptor")
SET(SHOP-SRCS
        ${CMAKE_SOURCE_DIR}/adaptor/shop-adaptor/shop-adaptor.c
)

INCLUDE_DIRECTORIES(
        ${CMAKE_SOURCE_DIR}/adaptor/shop-adaptor
)

INCLUDE(FindPkgConfig)
pkg_check_modules(shop_pkgs REQUIRED dlog glib-2.0)

set(EXTRA_CFLAGS "")
FOREACH(flag ${shop_pkgs_CFLAGS})
        SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS} -std=gnu99")

#message(">>> extra_cflags ${EXTRA_CFLAGS}")
#message(">>> result ${CMAKE_C_FLAGS}")
#message(">>> ld flag ${resource_pkgs_LDFLAGS}")

SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--as-needed")

ADD_LIBRARY(${SHOP-LIB} SHARED ${SHOP-SRCS})
TARGET_LINK_LIBRARIES(${SHOP-LIB} ${shop_pkgs_LDFLAGS})
SET_TARGET_PROPERTIES(${SHOP-LIB} PROPERTIES SOVERSION ${VERSION_MAJOR})
SET_TARGET_PROPERTIES(${SHOP-LIB} PROPERTIES VERSION ${VERSION})

#INSTALL(FILES ${CMAKE_SOURCE_DIR}/adaptor/shop-adaptor/shop-adaptor.h DESTINATION /usr/include/)
INSTALL(TARGETS ${SHOP-LIB} DESTINATION ${LIB_INSTALL_DIR} COMPONENT RuntimeLibraries)

SET(SHOP-ADAPTOR-HEADERS
        ${CMAKE_SOURCE_DIR}/adaptor/shop-adaptor/shop-adaptor.h
)
INSTALL(FILES ${SHOP-ADAPTOR-HEADERS} DESTINATION include/service-provider)
