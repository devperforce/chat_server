
cmake_minimum_required(VERSION 3.28)

set(PROJECT_NAME "chat_server")

if(MSVC)
    file(GLOB_RECURSE TARGET_DATA_TABLES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} config_tables/*json)
endif()

set(TARGET_SOURCES
    configs/server_setting.cpp
    configs/config_loader.cpp
    session/chat_session.cpp
    user_detail/user.cpp
    user_detail/user_repository.cpp
    user_detail/user_handler.cpp
    chat/chat_room.cpp
    chat/chat_room_selector.cpp
    chat/chat_handler.cpp
    handler/packet_handler_registry.cpp
    startup_validator.cpp
    chat_server.cpp
    main.cpp
    pch.cpp
)

set(TARGET_HEADERS
    configs/server_setting.h
    configs/config_loader.h
    content/content_type.h
    session/chat_session.h
    user_detail/user.h
    user_detail/user_repository.h
    user_detail/user_handler.h
    chat/chat_room.h
    chat/chat_room_selector.h
    chat/chat_handler.h
    handler/packet_handler_registry.h
    startup_validator.h
    chat_server.h
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
            ${iguana_SOURCE_DIR}
            ${iguana_SOURCE_DIR}/third_party/frozen/include
)

# library
set(PROJECT_LIBRARIES
    PRIVATE spdlog::spdlog_header_only
            ${Boost_LIBRARIES}
            magic_enum::magic_enum
            protobuf::libprotobuf
            system
            utility
            network
            protobuf
            database
            OpenSSL::Crypto
            OpenSSL::SSL
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

        # build_event
        add_custom_command(
            TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${CMAKE_CURRENT_SOURCE_DIR}/config_tables 
                $<TARGET_FILE_DIR:${PROJECT_NAME}>/config
        )
    endif()
endif()
