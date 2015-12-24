CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(service-adaptor)

SET(CMAKE_SKIP_BUILD_RPATH TRUE)
SET(VERSION_MAJOR 1)
SET(VERSION "${VERSION_MAJOR}.1.0")

IF("${CMAKE_BUILD_TYPE}" STREQUAL "")
        SET(CMAKE_BUILD_TYPE "Debug")
ENDIF("${CMAKE_BUILD_TYPE}" STREQUAL "")

MESSAGE("")
MESSAGE(">>> current directory: ${CMAKE_CURRENT_SOURCE_DIR}")
MESSAGE(">>> Build type: ${CMAKE_BUILD_TYPE}")

##########################################################
# Define Adaptor
##########################################################
#ADD_SUBDIRECTORY(adaptor)

##########################################################
# Define BASE LIB
##########################################################
ADD_SUBDIRECTORY(common)

##########################################################
# Define Adaptor API
##########################################################
ADD_SUBDIRECTORY(api)

##########################################################
# Define Adaptor Manager & Execute File
##########################################################
#ADD_SUBDIRECTORY(server)

##########################################################
# Define Test App
##########################################################
#ADD_SUBDIRECTORY(test)

##########################################################
# Define Install Files
##########################################################

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/packaging/org.tizen.serviceadaptor.client.conf.in ${CMAKE_CURRENT_SOURCE_DIR}/org.tizen.serviceadaptor.client.conf)
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/org.tizen.serviceadaptor.client.conf DESTINATION /etc/dbus-1/system.d)

CONFIGURE_FILE(service-adaptor.pc.in service-adaptor.pc @ONLY)
INSTALL(FILES ${CMAKE_BINARY_DIR}/service-adaptor.pc DESTINATION ${LIB_INSTALL_DIR}/pkgconfig)

# LICENSE file
INSTALL(FILES ${CMAKE_SOURCE_DIR}/LICENSE.APLv2 DESTINATION /usr/share/license/service-adaptor)
