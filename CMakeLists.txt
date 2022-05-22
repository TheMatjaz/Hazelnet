# Copyright © 2020-2022, Matjaž Guštin <dev@matjaz.it>
# <https://matjaz.it>. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice,
#    this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
# 3. Neither the name of nor the names of its contributors may be used to
#    endorse or promote products derived from this software without specific
#    prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS “AS IS”
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
# THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# -----------------------------------------------------------------------------
# CMake configuration
# -----------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.14)
project(Hazelnet
        LANGUAGES C
        VERSION 3.0.1
        DESCRIPTION
        "Reference implementation of the CAN Bus Security (CBS) protocol")


# -----------------------------------------------------------------------------
# Global settings
# -----------------------------------------------------------------------------
# Default build type, if not specified explicitly with `-DCMAKE_BUILD_TYPE`
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE MinSizeRel)
    message("CMAKE_BUILD_TYPE unspecified, defaulting to ${CMAKE_BUILD_TYPE}")
endif ()

set(CMAKE_EXPORT_COMPILE_COMMANDS OFF)

# Windows Crypto library needs to be explicitly linked to get secure
# random number generation. On Unix is as easy as reading /dev/urandom,
# so stdio.h suffices.
if (WIN32 OR MSYS)
    set(USE_BCRYPT TRUE)
endif ()
message("Using bcrypt: ${USE_BCRYPT}")


# -----------------------------------------------------------------------------
# Compiler flags
# -----------------------------------------------------------------------------
include(toolsupport/cmake/compiler_flags.cmake)


# -----------------------------------------------------------------------------
# Common library source files
# -----------------------------------------------------------------------------
set(LIB_HZL_COMMON_SRC_ANY_PLATFORM
        src/common/hzl_CommonAead.c
        src/common/hzl_CommonAead.h
        src/common/hzl_CommonEndian.c
        src/common/hzl_CommonEndian.h
        src/common/hzl_CommonHash.c
        src/common/hzl_CommonHash.h
        src/common/hzl_CommonHeader.c
        src/common/hzl_CommonHeader.h
        src/common/hzl_CommonInternal.h
        src/common/hzl_CommonPacking.c
        src/common/hzl_CommonPayload.h
        src/common/hzl_CommonTimeDelta.c
        src/common/hzl_CommonUtils.c
        src/common/hzl_CommonBuildUnsecured.c
        src/common/hzl_CommonBuildSecuredFd.c
        src/common/hzl_CommonMessage.h
        src/common/hzl_CommonBuildRequest.c
        src/common/hzl_CommonBuildResponse.c
        src/common/hzl_CommonProcessReceivedUnsecured.c
        src/common/hzl_CommonCtrDelay.c)
set(LIB_HZL_COMMON_SRC_ON_OS
        ${LIB_HZL_COMMON_SRC_ANY_PLATFORM}
        src/common/hzl_CommonOsTime.c
        src/common/hzl_CommonOsTrng.c
        src/common/hzl_CommonOsNewMsg.c
        )


# -----------------------------------------------------------------------------
# Common build targets
# -----------------------------------------------------------------------------

include(toolsupport/cmake/doxygen.cmake)

add_subdirectory(external/libascon)

# Copy all library API headers into the build target folder.
add_custom_target(hzl_copy_header_files ALL  # ALL to run it on make-all
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/inc/
        ${CMAKE_BINARY_DIR}/inc/
        )
set_target_properties(hzl_copy_header_files PROPERTIES
        ADDITIONAL_CLEAN_FILES ${CMAKE_BINARY_DIR}/inc/
        )


# -----------------------------------------------------------------------------
# Client library source files
# -----------------------------------------------------------------------------
set(LIB_HZL_CLIENT_SRC_ANY_PLATFORM
        ${LIB_HZL_COMMON_SRC_ANY_PLATFORM}
        src/client/hzl_ClientDeinit.c
        src/client/hzl_ClientInit.c
        src/client/hzl_ClientBuildUnsecured.c
        src/client/hzl_ClientBuildSecuredFd.c
        src/client/hzl_ClientGroup.c
        src/client/hzl_ClientProcessReceived.c
        src/client/hzl_ClientProcessReceived.h
        src/client/hzl_ClientProcessReceivedSecuredFd.c
        src/client/hzl_ClientProcessReceivedSecuredTp.c
        src/client/hzl_ClientProcessReceivedResponse.c
        src/client/hzl_ClientProcessReceivedRenewal.c
        src/client/hzl_ClientBuildRequest.c
        src/client/hzl_ClientInternal.h
        )
# Superset of Client source files including functionality for a desktop OS
set(LIB_HZL_CLIENT_SRC_ON_OS
        ${LIB_HZL_CLIENT_SRC_ANY_PLATFORM}
        ${LIB_HZL_COMMON_SRC_ON_OS}
        src/client/hzl_ClientNew.c
        src/client/hzl_ClientFree.c
        src/client/hzl_ClientNewMsg.c
        )


# -----------------------------------------------------------------------------
# Client library build targets
# -----------------------------------------------------------------------------

# Static library for any system, including embedded
add_library(hzl_client_any STATIC
        ${LIB_HZL_CLIENT_SRC_ANY_PLATFORM}
        )
add_dependencies(hzl_client_any
        ascon128hash
        hzl_copy_header_files
        )
target_include_directories(hzl_client_any
        PUBLIC inc/
        PRIVATE src/common/
        PRIVATE src/client/
        PRIVATE external/libascon/inc/
        )
target_link_libraries(hzl_client_any
        PRIVATE ascon128hash
        )


# Static library for desktop system
add_library(hzl_client_desktop STATIC
        ${LIB_HZL_CLIENT_SRC_ON_OS}
        )
add_dependencies(hzl_client_desktop
        ascon128hash
        hzl_copy_header_files
        )
target_include_directories(hzl_client_desktop
        PUBLIC inc/
        PRIVATE src/common/
        PRIVATE src/client/
        PRIVATE external/libascon/inc/
        )
target_link_libraries(hzl_client_desktop
        PRIVATE ascon128hash
        )
if (USE_BCRYPT)
    target_link_libraries(hzl_client_desktop
            PRIVATE bcrypt
            )
endif ()


# Shared library for desktop system
add_library(hzl_client_desktop_shared SHARED
        ${LIB_HZL_CLIENT_SRC_ON_OS}
        )
add_dependencies(hzl_client_desktop_shared
        ascon128hash
        hzl_copy_header_files
        )
target_include_directories(hzl_client_desktop_shared
        PUBLIC inc/
        PRIVATE src/common/
        PRIVATE src/client/
        PRIVATE external/libascon/inc/
        )
target_link_libraries(hzl_client_desktop_shared
        PRIVATE ascon128hash
        )
if (USE_BCRYPT)
    target_link_libraries(hzl_client_desktop_shared
            PRIVATE bcrypt
            )
endif ()
set_target_properties(hzl_client_desktop_shared PROPERTIES
        INTERPROCEDURAL_OPTIMIZATION True
        POSITION_INDEPENDENT_CODE True
        # Remove any "msys-" and enforce the same lib name with all toolchains
        PREFIX lib
        OUTPUT_NAME hzl_client
        )


# -----------------------------------------------------------------------------
# Server library source files
# -----------------------------------------------------------------------------
set(LIB_HZL_SERVER_SRC_ANY_PLATFORM
        ${LIB_HZL_COMMON_SRC_ANY_PLATFORM}
        src/server/hzl_ServerBuildSecuredFd.c
        src/server/hzl_ServerBuildUnsecured.c
        src/server/hzl_ServerDeInit.c
        src/server/hzl_ServerInit.c
        src/server/hzl_ServerNew.c
        src/server/hzl_ServerFree.c
        src/server/hzl_ServerInternal.h
        src/server/hzl_ServerProcessReceived.c
        src/server/hzl_ServerGroup.c
        src/server/hzl_ServerProcessReceivedRequest.c
        src/server/hzl_ServerProcessReceived.h
        src/server/hzl_ServerRenewalPhase.c
        src/server/hzl_ServerProcessReceivedSecuredFd.c
        src/server/hzl_ServerForceSessionRenewal.c
        )
# Superset of Server source files including functionality for a desktop OS
set(LIB_HZL_SERVER_SRC_ON_OS
        ${LIB_HZL_COMMON_SRC_ON_OS}
        ${LIB_HZL_SERVER_SRC_ANY_PLATFORM}
        src/server/hzl_ServerNewMsg.c
        )


# -----------------------------------------------------------------------------
# Server library build targets
# -----------------------------------------------------------------------------

# Static library for any system, including embedded
add_library(hzl_server_any STATIC
        ${LIB_HZL_SERVER_SRC_ANY_PLATFORM}
        )
add_dependencies(hzl_server_any
        ascon128hash
        hzl_copy_header_files
        )
target_include_directories(hzl_server_any
        PUBLIC inc/
        PRIVATE src/common/
        PRIVATE src/server/
        PRIVATE external/libascon/inc/
        )
target_link_libraries(hzl_server_any
        PRIVATE ascon128hash
        )


# Static library for desktop system
add_library(hzl_server_desktop STATIC
        ${LIB_HZL_SERVER_SRC_ON_OS}
        )
add_dependencies(hzl_server_desktop
        ascon128hash
        hzl_copy_header_files
        )
target_include_directories(hzl_server_desktop
        PUBLIC inc/
        PRIVATE src/common/
        PRIVATE src/server/
        PRIVATE external/libascon/inc/
        )
target_link_libraries(hzl_server_desktop
        PRIVATE ascon128hash
        )
if (USE_BCRYPT)
    target_link_libraries(hzl_server_desktop
            PRIVATE bcrypt
            )
endif ()


# Shared library for desktop system
add_library(hzl_server_desktop_shared SHARED
        ${LIB_HZL_SERVER_SRC_ON_OS}
        )
add_dependencies(hzl_server_desktop_shared
        ascon128hash
        hzl_copy_header_files
        )
target_include_directories(hzl_server_desktop_shared
        PUBLIC inc/
        PRIVATE src/common/
        PRIVATE src/server/
        PRIVATE external/libascon/inc/
        )
target_link_libraries(hzl_server_desktop_shared
        PRIVATE ascon128hash
        )
if (USE_BCRYPT)
    target_link_libraries(hzl_server_desktop_shared
            PRIVATE bcrypt
            )
endif ()
set_target_properties(hzl_server_desktop_shared PROPERTIES
        INTERPROCEDURAL_OPTIMIZATION True
        POSITION_INDEPENDENT_CODE True
        # Remove any "msys-" and enforce the same lib name with all toolchains
        PREFIX lib
        OUTPUT_NAME hzl_server
        )


# -----------------------------------------------------------------------------
# Test runners common source files
# -----------------------------------------------------------------------------
set(LIB_ATTO
        external/atto/src/atto.h
        external/atto/src/atto.c
        )
set(TEST_HZL_COMMON_SRC
        ${LIB_ATTO}
        tst/hzlTest.h
        tst/hzlTest_IoMockup.c
        )

# -----------------------------------------------------------------------------
# Test runners source files to test Client library
# -----------------------------------------------------------------------------
set(TEST_HZL_CLIENT_SRC
        ${TEST_HZL_COMMON_SRC}
        tst/client/hzlClientTest_BuildRequest.c
        tst/client/hzlClientTest_BuildSecuredFd.c
        tst/client/hzlClientTest_BuildUnsecured.c
        tst/client/hzlClientTest_Constants.c
        tst/client/hzlClientTest_DeInit.c
        tst/client/hzlClientTest_Init.c
        tst/client/hzlClientTest_InitCheckClientConfig.c
        tst/client/hzlClientTest_InitCheckGroupConfigs.c
        tst/client/hzlClientTest_InitCheckIo.c
        tst/client/hzlClientTest_Main.c
        tst/client/hzlClientTest_New.c
        tst/client/hzlClientTest_NewMsg.c
        tst/client/hzlClientTest_ProcessReceived.c
        tst/client/hzlClientTest_ProcessReceivedRenewal.c
        tst/client/hzlClientTest_ProcessReceivedRequest.c
        tst/client/hzlClientTest_ProcessReceivedResponse.c
        tst/client/hzlClientTest_ProcessReceivedSecuredFd.c
        tst/client/hzlClientTest_ProcessReceivedUnsecured.c
        )


# -----------------------------------------------------------------------------
# Test runners for Client library build targets
# -----------------------------------------------------------------------------

# Copy the static configuration files used to test the configuration loading
# functions in the test suite build directory, so the relative links
# within the test suite keep working properly.
add_custom_target(hzl_copy_client_config_files ALL  # ALL to run it on make-all
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/tst/client/clientconfigfiles/
        ${CMAKE_BINARY_DIR}/clientconfigfiles/
        )
set_target_properties(hzl_copy_client_config_files PROPERTIES
        ADDITIONAL_CLEAN_FILES ${CMAKE_BINARY_DIR}/clientconfigfiles/
        )

# Test runner executable for desktop using the static library
add_executable(test_hzl_client_desktop ${TEST_HZL_CLIENT_SRC})
add_dependencies(test_hzl_client_desktop
        hzl_client_desktop
        hzl_copy_client_config_files
        )
target_include_directories(test_hzl_client_desktop
        PRIVATE inc/
        PRIVATE tst/
        PRIVATE tst/client/
        PRIVATE external/atto/src/
        )
target_link_libraries(test_hzl_client_desktop
        PRIVATE hzl_client_desktop
        )

# Test runner executable for desktop using the shared library
add_executable(test_hzl_client_desktop_shared ${TEST_HZL_CLIENT_SRC})
add_dependencies(test_hzl_client_desktop_shared
        hzl_client_desktop_shared
        hzl_copy_client_config_files
        )
target_include_directories(test_hzl_client_desktop_shared
        PRIVATE inc/
        PRIVATE tst/
        PRIVATE tst/client/
        PRIVATE external/atto/src/
        )
target_link_libraries(test_hzl_client_desktop_shared
        PRIVATE hzl_client_desktop_shared
        )

# ctest enabled to run the test executables
enable_testing()
add_test(NAME test_hzl_client_desktop
        COMMAND test_hzl_client_desktop)
add_test(NAME test_hzl_client_desktop_shared
        COMMAND test_hzl_client_desktop_shared)


# -----------------------------------------------------------------------------
# Test runners source files to test Server library
# -----------------------------------------------------------------------------
set(TEST_HZL_SERVER_SRC
        ${TEST_HZL_COMMON_SRC}
        tst/server/hzlServerTest_Constants.c
        tst/server/hzlServerTest_Init.c
        tst/server/hzlServerTest_InitCheckServerConfig.c
        tst/server/hzlServerTest_Main.c
        tst/server/hzlServerTest_InitCheckClientConfig.c
        tst/server/hzlServerTest_InitCheckGroupConfigs.c
        tst/server/hzlServerTest_InitCheckIo.c
        tst/server/hzlServerTest_DeInit.c
        tst/server/hzlServerTest_New.c
        tst/server/hzlServerTest_BuildUnsecured.c
        tst/server/hzlServerTest_BuildSecuredFd.c
        tst/server/hzlServerTest_ProcessReceived.c
        tst/server/hzlServerTest_ProcessReceivedRequest.c
        tst/server/hzlServerTest_ProcessReceivedServerOnlyMsg.c
        tst/server/hzlServerTest_ProcessReceivedUnsecured.c
        tst/server/hzlServerTest_ProcessReceivedSecuredFd.c
        tst/server/hzlServerTest_ForceSessionRenewal.c
        )


# -----------------------------------------------------------------------------
# Test runners for Server library build targets
# -----------------------------------------------------------------------------

# Copy the static configuration files used to test the configuration loading
# functions in the test suite build directory, so the relative links
# within the test suite keep working properly.
add_custom_target(hzl_copy_server_config_files ALL  # ALL to run it on make-all
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/tst/server/serverconfigfiles/
        ${CMAKE_BINARY_DIR}/serverconfigfiles/
        )
set_target_properties(hzl_copy_server_config_files PROPERTIES
        ADDITIONAL_CLEAN_FILES ${CMAKE_BINARY_DIR}/serverconfigfiles/
        )

# Test runner executable for desktop using the static library0
add_executable(test_hzl_server_desktop ${TEST_HZL_SERVER_SRC})
add_dependencies(test_hzl_server_desktop
        hzl_server_desktop
        hzl_copy_server_config_files
        )
target_include_directories(test_hzl_server_desktop
        PRIVATE inc/
        PRIVATE tst/
        PRIVATE tst/server/
        PRIVATE external/atto/src/
        )
target_link_libraries(test_hzl_server_desktop
        PRIVATE hzl_server_desktop
        )

# Test runner executable for desktop using the shared library
add_executable(test_hzl_server_desktop_shared ${TEST_HZL_SERVER_SRC})
add_dependencies(test_hzl_server_desktop_shared
        hzl_server_desktop_shared
        hzl_copy_server_config_files
        )
target_include_directories(test_hzl_server_desktop_shared
        PRIVATE inc/
        PRIVATE tst/
        PRIVATE tst/server/
        PRIVATE external/atto/src/
        )
target_link_libraries(test_hzl_server_desktop_shared
        PRIVATE hzl_server_desktop_shared
        )

# ctest enabled to run the test executables
enable_testing()
add_test(NAME test_hzl_server_desktop
        COMMAND test_hzl_server_desktop)
add_test(NAME test_hzl_server_desktop_shared
        COMMAND test_hzl_server_desktop_shared)


# -----------------------------------------------------------------------------
# Test runners source files to test interoperability between Client and Server
# -----------------------------------------------------------------------------
set(TEST_HZL_INTEROP_SRC
        ${TEST_HZL_COMMON_SRC}
        tst/interop/hzlInteropTest_Main.c
        )


# -----------------------------------------------------------------------------
# Test runners to test interoperability between Client and Server
# -----------------------------------------------------------------------------
# Test runner executable for desktop using the static libraries
add_executable(test_hzl_interop_desktop ${TEST_HZL_INTEROP_SRC})
add_dependencies(test_hzl_interop_desktop
        hzl_client_desktop
        hzl_server_desktop
        hzl_copy_client_config_files
        hzl_copy_server_config_files
        )
target_include_directories(test_hzl_interop_desktop
        PRIVATE inc/
        PRIVATE tst/
        PRIVATE tst/interop/
        PRIVATE external/atto/src/
        )
target_link_libraries(test_hzl_interop_desktop
        PRIVATE hzl_client_desktop
        PRIVATE hzl_server_desktop
        )

# Test runner executable for desktop using the shared library
add_executable(test_hzl_interop_desktop_shared ${TEST_HZL_INTEROP_SRC})
add_dependencies(test_hzl_interop_desktop_shared
        hzl_client_desktop_shared
        hzl_server_desktop_shared
        hzl_copy_client_config_files
        hzl_copy_server_config_files
        )
target_include_directories(test_hzl_interop_desktop_shared
        PRIVATE inc/
        PRIVATE tst/
        PRIVATE tst/interop/
        PRIVATE external/atto/src/
        )
target_link_libraries(test_hzl_interop_desktop_shared
        PRIVATE hzl_client_desktop_shared
        PRIVATE hzl_server_desktop_shared
        )

# ctest enabled to run the test executables
enable_testing()
add_test(NAME test_hzl_interop_desktop
        COMMAND test_hzl_interop_desktop)
add_test(NAME test_hzl_interop_desktop_shared
        COMMAND test_hzl_interop_desktop_shared)
