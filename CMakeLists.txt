cmake_minimum_required(VERSION 3.1)

message(STATUS "CMake version: ${CMAKE_VERSION}")
message(STATUS "CMake build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "freeze CMAKE_CURRENT_SOURCE_DIR: ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "freeze PROJECT_BINARY_DIR: ${PROJECT_BINARY_DIR}")

# Note: update this to your new project's name and version
project(
  freeze
  VERSION 1.0.0
  LANGUAGES C CXX
)


# ---- Include guards ----

if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
  message(
    FATAL_ERROR
      "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there."
  )
endif()


set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)

message(STATUS "freeze CMAKE_RUNTIME_OUTPUT_DIRECTORY: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
message(STATUS "freeze CMAKE_LIBRARY_OUTPUT_DIRECTORY: ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
message(STATUS "freeze CMAKE_ARCHIVE_OUTPUT_DIRECTORY: ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}")

SET(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS ON)

# Debug
set(CMAKE_BUILD_TYPE Debug)
# set(CMAKE_BUILD_TYPE RelWithDebInfo)

message(STATUS "CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")

#PROJECT_SOURCE_DIR
#CMAKE_CURRENT_LIST_DIR
#set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
#set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

set(IncludeDirs)
set(LinkDirs)
set(LinkLibs)

message(STATUS "WIN32: ${WIN32}")

# 定义宏：自动为源文件创建与磁盘一致的VS过滤器结构
# 自动分组宏
macro(auto_source_group SOURCE_FILES)
    foreach(FILE ${SOURCE_FILES})
        # 获取文件相对于项目根目录的路径
        file(RELATIVE_PATH RELATIVE_FILE ${CMAKE_CURRENT_SOURCE_DIR} ${FILE})

        # 提取目录路径并替换分隔符
        get_filename_component(DIR_PATH ${RELATIVE_FILE} DIRECTORY)
        if(DIR_PATH)
            string(REPLACE "/" "\\" GROUP_PATH ${DIR_PATH})
            source_group(${GROUP_PATH} FILES ${FILE})
        else()
            source_group("\\" FILES ${FILE})  # 根目录文件
        endif()

    endforeach()
endmacro()

if (WIN32)
	# set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT") 
	# set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")

    set(CompilerFlags
            CMAKE_CXX_FLAGS
            CMAKE_CXX_FLAGS_DEBUG
            CMAKE_CXX_FLAGS_RELEASE
            CMAKE_CXX_FLAGS_MINSIZEREL
            CMAKE_C_FLAGS
            CMAKE_C_FLAGS_DEBUG
            CMAKE_C_FLAGS_RELEASE
            CMAKE_C_FLAGS_MINSIZEREL
            )

    foreach(CompilerFlag ${CompilerFlags})
        string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
    endforeach ()

    set(IncludeDirs
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/OpenSSL-Win64/include
        "${CMAKE_CURRENT_SOURCE_DIR}/third_party/MySQL/MySQL Server 5.7/include"
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
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/croncpp/include
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/http-parser
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/libnats
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/fmt/include)

    set(LinkDirs
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/libs
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/OpenSSL-Win64/lib
        "${CMAKE_CURRENT_SOURCE_DIR}/third_party/MySQL/MySQL Server 5.7/lib")

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

    message(STATUS "IncludeDirs: ${IncludeDirs}")
    message(STATUS "LinkDirs: ${LinkDirs}")
    message(STATUS "LinkLibs: ${LinkLibs}")

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
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/croncpp/include
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/http-parser
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/JPSPlusGoalBounding
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/jsoncpp/include
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/tinyxml2
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/influxdb-cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/json/include
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/pfr/include
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/fmt/include
        ${CMAKE_CURRENT_SOURCE_DIR}/pb_msg
        ${CMAKE_CURRENT_SOURCE_DIR}/apie)

    set(LinkDirs
        /usr/lib64/mysql
        /usr/local/lib64
        /usr/local/lib
        /usr/local/protobuf/lib
        /usr/local/apie/lib)

    set(LinkLibs -lmysqlclient -levent -levent_pthreads -ldl -lc -lrt -lcrypto -lssl -llz4 -lpthread)

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -Wall -W -Wno-narrowing -Wno-unused-parameter -Wno-deprecated-declarations -fmessage-length=0 -fPIC -pthread -Wextra -fpermissive -rdynamic ${M64FLAG}")
endif()

message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

message(STATUS "freeze PROJECT_SOURCE_DIR: ${PROJECT_SOURCE_DIR}")
message(STATUS "freeze PROJECT_BINARY_DIR: ${PROJECT_BINARY_DIR}")


# -----------------------------------------------------------------------------
# ----- LIBRARY ---------------------------------------------------------
# ----------------------------------------------------------------------------- 


add_subdirectory(apie)
add_subdirectory(lib_pb)
add_subdirectory(server)

message(STATUS "CMAKE_INSTALL_BINDIR: ${CMAKE_INSTALL_BINDIR}")
message(STATUS "CMAKE_INSTALL_LIBDIR: ${CMAKE_INSTALL_LIBDIR}")

install(TARGETS apie pb db_proxy_server gateway_server login_server scene_server service_registry test_server
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