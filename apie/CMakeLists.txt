message(STATUS "apie")
message(STATUS "apie CMAKE_CURRENT_SOURCE_DIR: ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "apie PROJECT_SOURCE_DIR: ${PROJECT_SOURCE_DIR}")
message(STATUS "apie CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")


# Set lists to empty beforehand as to not accidentally take values from parent
set(public_headers)
set(sources)
set(cxx-outer-sources)
set(cxx-include-dirs)
set(cxx-link-dirs)

set(cxx-include-dirs ${IncludeDirs})
set(cxx-link-dirs ${LinkDirs})

message(STATUS "apie cxx-include-dirs: ${cxx-include-dirs}")


include_directories(${cxx-include-dirs})
link_directories(${cxx-link-dirs})


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
target_link_libraries(apie ${LinkLibs})


message(STATUS "apie PROJECT_BINARY_DIR: ${PROJECT_BINARY_DIR}")