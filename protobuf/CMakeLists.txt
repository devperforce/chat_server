cmake_minimum_required (VERSION 3.28)

set(PROJECT_NAME "protobuf")

set(CMAKE_INSTALL_DEFAULT_DIRECTORY_PERMISSIONS
    OWNER_READ
    OWNER_WRITE
    OWNER_EXECUTE
    GROUP_READ
)

set(TARGET_SOURCES
    generated/packet_id.pb.cc
    generated/chatting.pb.cc
    protobuf_session.cpp
    pch.cpp
)

set(TARGET_HEADERS
    handler/handler.h
    handler/register_handler.h
    generated/packet_id.pb.h
    generated/chatting.pb.h
    protobuf_session.h
    pch.h
)

if(MSVC)
    file(GLOB_RECURSE TARGET_PROTOS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} idl/*proto)
endif()

add_library(${PROJECT_NAME} STATIC
   ${TARGET_SOURCES}
   ${TARGET_HEADERS}
   ${TARGET_PROTOS}
)

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS_RELEASE -s)
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER ${PACKET_FOLDER_NAME})

if(MSVC)

    if ("${CMAKE_MAKE_PROGRAM}" MATCHES "MSBuild")
        # filter
        foreach(FILE ${TARGET_SOURCES} ${TARGET_HEADERS} ${TARGET_PROTOS}) 
            get_filename_component(PARENT_DIR "${FILE}" PATH)
            string(REPLACE "/" "\\" GROUP "${PARENT_DIR}")
            if ("${FILE}" MATCHES ".*\\.cpp")
                set(GROUP "${GROUP}")
            elseif("${FILE}" MATCHES ".*\\.hpp")
                set(GROUP "${GROUP}")
            elseif("${FILE}" MATCHES ".*\\.h")
                set(GROUP "${GROUP}")
            elseif("${FILE}" MATCHES ".*\\.proto")
                set(GROUP "${GROUP}")
            endif()
            source_group("${GROUP}" FILES "${FILE}")
        endforeach()
    endif()
endif()
