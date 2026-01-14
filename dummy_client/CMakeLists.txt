
cmake_minimum_required(VERSION 3.28)

set(PROJECT_NAME "dummy_client")

#[[
if(MSVC)
    file(GLOB_RECURSE TARGET_DATA_TABLES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} config_tables/*json)
endif()
]]

#file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/script DESTINATION ${CMAKE_BINARY_DIR}/binaries/${PROJECT_NAME})

set(TARGET_SOURCES
    handler/connection_handler.cpp
    public_chat/chat_handler.cpp
    dummy_session.cpp
    main.cpp
    pch.cpp
)

set(TARGET_HEADERS
    handler/connection_handler.h
    public_chat/chat_handler.h
    dummy_session.h
    pch.h
)

# executable
add_executable(${PROJECT_NAME}
    ${TARGET_SOURCES}
    ${TARGET_HEADERS}
    ${TARGET_DATA_TABLES}
)

# include header
target_include_directories(
    ${PROJECT_NAME}
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)

# library
set(PROJECT_LIBRARIES
    PRIVATE spdlog::spdlog_header_only
            ${Boost_LIBRARIES}
            #date::date date::date-tz
            magic_enum::magic_enum
            protobuf::libprotobuf
            #redis++::redis++_static
            system
            utility
            network
            protobuf
)

if(UNIX)
    target_link_libraries(${PROJECT_NAME} ${PROJECT_LIBRARIES})
elseif(MSVC)
    target_link_libraries(${PROJECT_NAME} ${PROJECT_LIBRARIES} PRIVATE odbc32.lib odbccp32.lib)
endif()

set_target_properties(${PROJECT_NAME}
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/binaries/$<CONFIG>/${PROJECT_NAME}
    FOLDER ${SERVER_FOLDER_NAME}
)

if(MSVC)
    if(COMMAND target_precompile_headers)
        target_precompile_headers (${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/pch.h)
    endif()
    if ("${CMAKE_MAKE_PROGRAM}" MATCHES "MSBuild")
        # working directory
        set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY $<TARGET_FILE_DIR:${PROJECT_NAME}>)
        # command arguments
        set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_COMMAND_ARGUMENTS run)
        
        # filter
        foreach(FILE ${TARGET_SOURCES} ${TARGET_HEADERS} ${TARGET_DATA_TABLES}) 
            get_filename_component(PARENT_DIR "${FILE}" PATH)
            string(REPLACE "/" "\\" GROUP "${PARENT_DIR}")
            if ("${FILE}" MATCHES ".*\\.cpp")
                set(GROUP "${GROUP}")
            elseif("${FILE}" MATCHES ".*\\.hpp")
                set(GROUP "${GROUP}")
            elseif("${FILE}" MATCHES ".*\\.h")
                set(GROUP "${GROUP}")
            elseif("${FILE}" MATCHES ".*\\.json")
                set(GROUP "${GROUP}")
            endif()
            source_group("${GROUP}" FILES "${FILE}")
        endforeach()
    endif()
endif()
