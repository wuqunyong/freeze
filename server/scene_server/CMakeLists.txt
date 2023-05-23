message(STATUS "scene_server")
message(STATUS "scene_server CMAKE_CURRENT_SOURCE_DIR: ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "scene_server PROJECT_SOURCE_DIR: ${PROJECT_SOURCE_DIR}")

message(STATUS "scene_server CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
message(STATUS "scene_server CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}")



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
        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib_pb)

    set(DependenciesLinkDirs
        ${PROJECT_BINARY_DIR}/lib)

	set(DependenciesLinkLibs -lapie -lpb -lprotobuf -lyaml-cpp -lnats -lcpp_redis -ltacopie)
endif()


file(GLOB_RECURSE SOURCE_FILES ./*.cc ./*.cpp)


add_executable(scene_server ${SOURCE_FILES})
target_include_directories(scene_server PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${IncludeDirs} ${DependenciesIncludeDirs})
target_link_directories(scene_server PUBLIC ${LinkDirs} ${DependenciesLinkDirs})
target_link_libraries(scene_server PUBLIC ${LinkLibs} ${DependenciesLinkLibs})