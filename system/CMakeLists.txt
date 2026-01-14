cmake_minimum_required (VERSION 3.28)

set(PROJECT_NAME "system")

set(CMAKE_INSTALL_DEFAULT_DIRECTORY_PERMISSIONS
        OWNER_READ
        OWNER_WRITE
        OWNER_EXECUTE
        GROUP_READ
)

set(TARGET_SOURCES
    actor.cpp
    system.cpp
    pch.cpp
)

set(TARGET_HEADERS
    actor.h
    custom_exception.h
    error_handler.h
    singleton.h
    framework.h
    type.h
    pch.h
)

add_library(${PROJECT_NAME} STATIC
   ${TARGET_SOURCES}
   ${TARGET_HEADERS}
)

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS_RELEASE -s)
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER ${ENGINE_FOLDER_NAME})

if(MSVC)
    if(COMMAND target_precompile_headers)
        target_precompile_headers (${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/pch.h)
    endif()
    if ("${CMAKE_MAKE_PROGRAM}" MATCHES "MSBuild")
        # filter
        foreach(FILE ${TARGET_SOURCES} ${TARGET_HEADERS}) 
            get_filename_component(PARENT_DIR "${FILE}" PATH)
            string(REPLACE "/" "\\" GROUP "${PARENT_DIR}")
            if ("${FILE}" MATCHES ".*\\.cpp")
                set(GROUP "${GROUP}")
            elseif("${FILE}" MATCHES ".*\\.hpp")
                set(GROUP "${GROUP}")
            elseif("${FILE}" MATCHES ".*\\.h")
                set(GROUP "${GROUP}")
            endif()
            source_group("${GROUP}" FILES "${FILE}")
        endforeach()
    endif()
endif()
