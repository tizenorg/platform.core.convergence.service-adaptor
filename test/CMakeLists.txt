CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(sal-test)

SET(CMAKE_SKIP_BUILD_RPATH TRUE)

IF("${CMAKE_BUILD_TYPE}" STREQUAL "")
        SET(CMAKE_BUILD_TYPE "Debug")
ENDIF("${CMAKE_BUILD_TYPE}" STREQUAL "")

MESSAGE("")
MESSAGE(">>> current directory: ${CMAKE_CURRENT_SOURCE_DIR}")
MESSAGE(">>> Build type: ${CMAKE_BUILD_TYPE}")

ADD_DEFINITIONS("-DSERVICE_ADAPTOR_DEBUGGING")

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${VISIBILITY} -fvisibility=hidden")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,--gc-sections")

SET(TEST-EXE "sal-test")
SET(TEST-SRCS
        ${CMAKE_SOURCE_DIR}/test/test.c
)

INCLUDE_DIRECTORIES(
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/client
)

INCLUDE(FindPkgConfig)
pkg_check_modules(test_pkgs REQUIRED dlog glib-2.0 capi-appfw-application)

FOREACH(flag ${test_pkgs_CFLAGS})
        SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS}")
#message(">>>[${TEST-EXE}] extra_cflags ${EXTRA_CFLAGS}")
#message(">>>[${TEST-EXE}] test_pkgs_cflags ${test_pkgs_CFLAGS}")
#message(">>>[${TEST-EXE}] result ${CMAKE_C_FLAGS}")

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS}")
SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--as-needed -pie")

ADD_EXECUTABLE(${TEST-EXE} ${TEST-SRCS})
TARGET_LINK_LIBRARIES(${TEST-EXE} ${test_pkgs_LDFLAGS} service-adaptor)
INSTALL(TARGETS ${TEST-EXE} DESTINATION bin)
