message(STATUS "apie")
message(STATUS "apie CMAKE_CURRENT_SOURCE_DIR: ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "apie PROJECT_SOURCE_DIR: ${PROJECT_SOURCE_DIR}")
message(STATUS "apie CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")


# Set lists to empty beforehand as to not accidentally take values from parent
set(public_headers)
set(sources)
set(cxx-outer-sources)
set(cxx-includes)
set(cxx-link)

set(cxx-includes
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/OpenSSL-Win64/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/MySQL/MySQLServer5.7/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/protobuf/src
    ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/yaml-cpp/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/libevent-release-2.1.11/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/libevent-release-2.1.11/WIN32-Code/nmake
    ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/influxdb-cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/lz4/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/jsoncpp/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/tinyxml2
    ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/cpp_redis/includes
    ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/cpp_redis/tacopie/includes
    ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/libnats)


set(cxx-link
    ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/libs
    ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/OpenSSL-Win64/lib
    ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/MySQL/MySQLServer5.7/lib)


include_directories(${cxx-includes})
link_directories(${cxx-link})


file(GLOB_RECURSE APIE_SOURCE ./*.cc ./*.cpp)

set(cxx-outer-sources
    third_party/http-parser/http_parser.c
    third_party/jsoncpp/src/lib_json/json_writer.cpp
    third_party/jsoncpp/src/lib_json/json_reader.cpp
    third_party/jsoncpp/src/lib_json/json_value.cpp
    third_party/tinyxml2/tinyxml2.cpp)


# -----------------------------------------------------------------------------
# source generators

foreach(source ${cxx-outer-sources})
  list(APPEND sources ${CMAKE_CURRENT_SOURCE_DIR}/../${source})
endforeach()


add_library(apie SHARED ${APIE_SOURCE} ${sources} ${public_headers})

set_target_properties(apie PROPERTIES VERSION "1.0.0" SOVERSION "1")
target_link_libraries(apie ws2_32.lib
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


message(STATUS "apie PROJECT_BINARY_DIR: ${PROJECT_BINARY_DIR}")