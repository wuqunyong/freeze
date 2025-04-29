message(STATUS "gateway_server")
message(STATUS "gateway_server CMAKE_CURRENT_SOURCE_DIR: ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "gateway_server PROJECT_SOURCE_DIR: ${PROJECT_SOURCE_DIR}")

message(STATUS "gateway_server CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
message(STATUS "gateway_server CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}")



# Set lists to empty beforehand as to not accidentally take values from parent

set(DependenciesIncludeDirs)
set(DependenciesLinkDirs)
set(DependenciesLinkLibs)
if (WIN32)
    set(DependenciesIncludeDirs
        ${CMAKE_CURRENT_SOURCE_DIR}/../../apie
        ${CMAKE_CURRENT_SOURCE_DIR}/../../server
        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib_pb)

    set(DependenciesLinkDirs
        ${PROJECT_BINARY_DIR}/lib)

    set(DependenciesLinkLibs 
        apie.lib
        pb.lib)
else ()
    set(DependenciesIncludeDirs
        ${CMAKE_CURRENT_SOURCE_DIR}/../../apie
        ${CMAKE_CURRENT_SOURCE_DIR}/../../server
        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib_pb)

    set(DependenciesLinkDirs
        ${PROJECT_BINARY_DIR}/lib
        /usr/local/apie/lib)

	set(DependenciesLinkLibs -lapie -lpb -lprotobuf -lyaml-cpp -lnats -lcpp_redis -ltacopie)
endif()


file(GLOB_RECURSE SOURCE_FILES ./*.cc ./*.cpp)
file(GLOB_RECURSE SOURCE_HEADER ./*.h)

add_executable(gateway_server ${SOURCE_FILES} ${SOURCE_HEADER})
target_include_directories(gateway_server PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${IncludeDirs} ${DependenciesIncludeDirs})
target_link_directories(gateway_server PUBLIC ${LinkDirs} ${DependenciesLinkDirs})
target_link_libraries(gateway_server PUBLIC ${LinkLibs} ${DependenciesLinkLibs})

if (WIN32)
    auto_source_group("${SOURCE_HEADER}")
    auto_source_group("${SOURCE_FILES}")
endif()