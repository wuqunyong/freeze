cmake_minimum_required(VERSION 3.1)

message(STATUS "CMake version: ${CMAKE_VERSION}")
message(STATUS "CMake build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "freeze CMAKE_CURRENT_SOURCE_DIR: ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "freeze PROJECT_BINARY_DIR: ${PROJECT_BINARY_DIR}")

# Project name
project(freeze)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++17")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)

# Debug
set(CMAKE_BUILD_TYPE Debug)
# set(CMAKE_BUILD_TYPE RelWithDebInfo)

message(STATUS "CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")

set(IncludeDirs)
set(LinkDirs)
set(LinkLibs)

if (WIN32)
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT") 
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")

    set(IncludeDirs
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/OpenSSL-Win64/include
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/MySQL/MySQLServer5.7/include
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/protobuf/src
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/yaml-cpp/include
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/libevent-release-2.1.11/include
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/libevent-release-2.1.11/WIN32-Code/nmake
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/influxdb-cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/lz4/include
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/jsoncpp/include
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/tinyxml2
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/cpp_redis/includes
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/cpp_redis/tacopie/includes
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/json/include
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/pfr/include
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/libnats)

    set(LinkDirs
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/libs
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/OpenSSL-Win64/lib
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/MySQL/MySQLServer5.7/lib)

	set(LinkLibs
        ws2_32.lib
        wsock32.lib
        libmysql.lib
        mysqlclient.lib
        yaml-cppd.lib
        event.lib
        event_core.lib
        event_extra.lib
        libprotobufd.lib
        libssl.lib
        libcrypto.lib
        liblz4_static.lib
        cpp_redis.lib
        tacopie.lib
        nats_staticd.lib)
else ()
    set(CMAKE_BUILD_TYPE RelWithDebInfo)

    set(CMAKE_INSTALL_PREFIX "/usr/local/apie")

    set(IncludeDirs
        /usr/local/include
        /usr/include/mysql
        /usr/local/protobuf/include
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/astar
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/cpp_redis/includes
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/cpp_redis/tacopie/includes
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/http-parser
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/JPSPlusGoalBounding
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/jsoncpp/include
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/tinyxml2
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/influxdb-cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/json/include
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/pfr/include
        ${CMAKE_CURRENT_SOURCE_DIR}/pb_msg
        ${CMAKE_CURRENT_SOURCE_DIR}/apie)

    set(LinkDirs
        /usr/lib64/mysql
        /usr/local/lib64
        /usr/local/lib
        /usr/local/protobuf/lib
        /usr/local/apie/lib)

    set(LinkLibs -lmysqlclient -levent -levent_pthreads -lpthread -ldl -lc -lrt -lcrypto -lssl -llz4 -lcpp_redis -ltacopie)
endif()

message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

message(STATUS "freeze PROJECT_SOURCE_DIR: ${PROJECT_SOURCE_DIR}")
message(STATUS "freeze PROJECT_BINARY_DIR: ${PROJECT_BINARY_DIR}")


# -----------------------------------------------------------------------------
# ----- LIBRARY ---------------------------------------------------------
# ----------------------------------------------------------------------------- 


add_subdirectory(apie)
add_subdirectory(server)

install(TARGETS apie db_proxy_server gateway_server login_server scene_server service_registry test_server
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(DIRECTORY conf/ DESTINATION conf/
        FILES_MATCHING PATTERN "*.yaml")
install(DIRECTORY data/ DESTINATION data/
        FILES_MATCHING PATTERN "*.*")
install(DIRECTORY etc/ DESTINATION etc/
        FILES_MATCHING PATTERN "*.pem"
        PATTERN "*.pub")
install(FILES
    "etc/apie.sh"
    DESTINATION "scripts")