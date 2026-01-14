cmake_minimum_required (VERSION 3.28)

set(PROJECT_NAME "utility")

set(CMAKE_INSTALL_DEFAULT_DIRECTORY_PERMISSIONS
        OWNER_READ
        OWNER_WRITE
        OWNER_EXECUTE
        GROUP_READ
)

# SET ( CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type (default Debug)" )
# SET ( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BUILD_TYPE} )
# SET ( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BUILD_TYPE}/lib )
# SET ( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BUILD_TYPE}/lib )

set(TARGET_SOURCES
    exception/exception_handler.cpp
    file/file_loader.h
    file/path.h
    string/string_converter.cpp
    log/static_logger.cpp
    log/dummy_logger.cpp
    log/logger.cpp
    log.cpp
    system_status.cpp
    utility.cpp
    pch.cpp
)

set(TARGET_HEADERS
    bitset/bitset_converter.h
    exception/exception_handler.h
    file/file_loader.cpp
    file/path.cpp
    string/string_converter.h
    math/float_comparison.h
    log/static_logger.h
    log/dummy_logger.h
    log/logger.h
    log.h
    system_status.h
    magic_enum_define.h
    framework.h
    pch.h
)

add_library(${PROJECT_NAME} STATIC
   ${TARGET_SOURCES}
   ${TARGET_HEADERS}
)

target_include_directories(
    ${PROJECT_NAME}
    PRIVATE ${iguana_SOURCE_DIR}
            ${iguana_SOURCE_DIR}/third_party/frozen/include
            ${CMAKE_CURRENT_SOURCE_DIR}
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
