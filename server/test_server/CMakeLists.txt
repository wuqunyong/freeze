message(STATUS "test_server")
message(STATUS "test_server CMAKE_CURRENT_SOURCE_DIR: ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "test_server PROJECT_SOURCE_DIR: ${PROJECT_SOURCE_DIR}")

message(STATUS "test_server CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
message(STATUS "test_server CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}")



# Set lists to empty beforehand as to not accidentally take values from parent
set(pb_sources)

set(DependenciesIncludeDirs)
set(DependenciesLinkDirs)
set(DependenciesLinkLibs)
if (WIN32)
    set(DependenciesIncludeDirs
        ${CMAKE_CURRENT_SOURCE_DIR}/../../apie)

    set(DependenciesLinkDirs
        ${PROJECT_BINARY_DIR}/lib)

    set(DependenciesLinkLibs 
        apie.lib)
else ()
endif()


set(pb_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/../../pb_msg/business/login_msg.pb.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/../../pb_msg/business/rpc_login.pb.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/../../pb_msg/business/role_server_msg.pb.cc)

file(GLOB_RECURSE SOURCE_FILES ./*.cc ./*.cpp)


add_executable(test_server ${SOURCE_FILES} ${pb_sources})
target_include_directories(test_server PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${IncludeDirs} ${DependenciesIncludeDirs})
target_link_directories(test_server PUBLIC ${LinkDirs} ${DependenciesLinkDirs})
target_link_libraries(test_server PUBLIC ${LinkLibs} ${DependenciesLinkLibs})