message(STATUS "pb")
message(STATUS "pb CMAKE_CURRENT_SOURCE_DIR: ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "pb PROJECT_SOURCE_DIR: ${PROJECT_SOURCE_DIR}")
message(STATUS "pb CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")


# Set lists to empty beforehand as to not accidentally take values from parent
set(public_headers)
set(sources)
set(cxx-outer-sources)

#set(cxx-include-dirs)
#set(cxx-link-dirs)
#set(cxx-include-dirs ${IncludeDirs})
#set(cxx-link-dirs ${LinkDirs})
#include_directories(${cxx-include-dirs})
#link_directories(${cxx-link-dirs})


set(DependenciesIncludeDirs)
set(DependenciesLinkDirs)
set(DependenciesLinkLibs)
if (WIN32)
    set(DependenciesIncludeDirs
        ${CMAKE_CURRENT_LIST_DIR})


else ()
    set(DependenciesIncludeDirs
        ${CMAKE_CURRENT_LIST_DIR})

endif()



file(GLOB_RECURSE PB_SOURCE ./*.cc ./*.cpp)
file(GLOB_RECURSE PB_HEADER ./*.h)

set(LibType)
if (WIN32)
    set(LibType STATIC)
else ()
	set(LibType SHARED)
endif()




add_library(pb ${LibType} ${PB_SOURCE} ${sources} ${public_headers} ${PB_HEADER})
set_target_properties(pb PROPERTIES VERSION "1.0.0" SOVERSION "1")
target_include_directories(pb PUBLIC ${IncludeDirs} ${DependenciesIncludeDirs})
target_link_directories(pb PUBLIC ${LinkDirs})
target_link_libraries(pb ${LinkLibs})


message(STATUS "pb PROJECT_BINARY_DIR: ${PROJECT_BINARY_DIR}")

if (WIN32)
    auto_source_group("${PB_HEADER}")
    auto_source_group("${PB_SOURCE}")
    auto_source_group("${sources}")
endif()