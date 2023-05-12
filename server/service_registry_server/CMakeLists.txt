message(STATUS "service_registry")
message(STATUS "service_registry CMAKE_CURRENT_SOURCE_DIR: ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "service_registry PROJECT_SOURCE_DIR: ${PROJECT_SOURCE_DIR}")

message(STATUS "service_registry CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
message(STATUS "service_registry CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}")



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

	set(DependenciesLinkLibs -lapie -lprotobuf -lyaml-cpp -lnats)
endif()


file(GLOB_RECURSE SOURCE_FILES ./*.cc ./*.cpp)


add_executable(service_registry ${SOURCE_FILES})
target_include_directories(service_registry PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${IncludeDirs} ${DependenciesIncludeDirs})
target_link_directories(service_registry PUBLIC ${LinkDirs} ${DependenciesLinkDirs})
target_link_libraries(service_registry PUBLIC ${LinkLibs} ${DependenciesLinkLibs})