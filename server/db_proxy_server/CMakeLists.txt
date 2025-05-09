message(STATUS "db_proxy_server")
message(STATUS "db_proxy_server CMAKE_CURRENT_SOURCE_DIR: ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "db_proxy_server PROJECT_SOURCE_DIR: ${PROJECT_SOURCE_DIR}")

message(STATUS "db_proxy_server CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
message(STATUS "db_proxy_server CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}")


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


message(STATUS "IncludeDirs: ${IncludeDirs}")
message(STATUS "LinkDirs: ${LinkDirs}")
message(STATUS "LinkLibs: ${LinkLibs}")
message(STATUS "DependenciesIncludeDirs: ${DependenciesIncludeDirs}")
message(STATUS "DependenciesLinkDirs: ${DependenciesLinkDirs}")
message(STATUS "DependenciesLinkLibs: ${DependenciesLinkLibs}")


add_executable(db_proxy_server ${SOURCE_FILES} ${common_sources} ${SOURCE_HEADER})
target_include_directories(db_proxy_server PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${IncludeDirs} ${DependenciesIncludeDirs})
target_link_directories(db_proxy_server PUBLIC ${LinkDirs} ${DependenciesLinkDirs})
target_link_libraries(db_proxy_server PUBLIC ${LinkLibs} ${DependenciesLinkLibs})

if (WIN32)
    auto_source_group("${SOURCE_HEADER}")
    auto_source_group("${SOURCE_FILES}")
endif()