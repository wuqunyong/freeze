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
        ${CMAKE_CURRENT_SOURCE_DIR}/../../apie)

    set(DependenciesLinkDirs
        ${PROJECT_BINARY_DIR}/lib)

	set(DependenciesLinkLibs 
        apie.lib
        pb.lib)
else ()
    set(DependenciesIncludeDirs
        ${CMAKE_CURRENT_SOURCE_DIR}/../../apie)

    set(DependenciesLinkDirs
        ${PROJECT_BINARY_DIR}/lib)

	set(DependenciesLinkLibs -lapie -lpb -lprotobuf -lyaml-cpp -lnats)
endif()


file(GLOB_RECURSE SOURCE_FILES ./*.cc ./*.cpp)


message(STATUS "IncludeDirs: ${IncludeDirs}")
message(STATUS "LinkDirs: ${LinkDirs}")
message(STATUS "LinkLibs: ${LinkLibs}")
message(STATUS "DependenciesIncludeDirs: ${DependenciesIncludeDirs}")
message(STATUS "DependenciesLinkDirs: ${DependenciesLinkDirs}")
message(STATUS "DependenciesLinkLibs: ${DependenciesLinkLibs}")


add_executable(db_proxy_server ${SOURCE_FILES} ${common_sources})
target_include_directories(db_proxy_server PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${IncludeDirs} ${DependenciesIncludeDirs})
target_link_directories(db_proxy_server PUBLIC ${LinkDirs} ${DependenciesLinkDirs})
target_link_libraries(db_proxy_server PUBLIC ${LinkLibs} ${DependenciesLinkLibs})