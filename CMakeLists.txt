cmake_minimum_required(VERSION 3.28)

include(FetchContent)

project(server_all)

#set(Boost_NO_WARN_NEW_VERSIONS 1)

set(Boost_USE_STATIC_LIBS ON)  # only find static libs
set(Boost_USE_MULTITHREADED ON)
if(MSVC)
    set(Boost_USE_STATIC_RUNTIME ON)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# 대분류 카테고리 속성
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# 대분류 카테고리 필터 이름
set(ENGINE_FOLDER_NAME "Engine")
set(NETWORK_FOLDER_NAME "Network")
set(PACKET_FOLDER_NAME "Packet")
set(DATABASE_FOLDER_NAME "Database")
set(SERVER_FOLDER_NAME "Server")
set(TOOL_FOLDER_NAME "Tool")

# 컴파일 옵션
set(CMAKE_CXX_STANDARD 23)
set(CXX_STANDARD_REQUIRED YES)
set(CXX_EXTENSIONS NO)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_INSTALL_DEFAULT_DIRECTORY_PERMISSIONS
    OWNER_READ
    OWNER_WRITE
    OWNER_EXECUTE
    GROUP_READ)
 
if(MSVC)
    macro(get_WIN32_WINNT version)
        if(CMAKE_SYSTEM_VERSION)
            set(ver ${CMAKE_SYSTEM_VERSION})
            string(REGEX MATCH "^([0-9]+).([0-9])" ver ${ver})
            string(REGEX MATCH "^([0-9]+)" verMajor ${ver})
            # Check for Windows 10, b/c we'll need to convert to hex 'A'.
            if("${verMajor}" MATCHES "10")
                set(verMajor "A")
                string(REGEX REPLACE "^([0-9]+)" ${verMajor} ver ${ver})
            endif()
            # Remove all remaining '.' characters.
            string(REPLACE "." "" ver ${ver})
            # Prepend each digit with a zero.
            string(REGEX REPLACE "([0-9A-Z])" "0\\1" ver ${ver})
            set(${version} "0x${ver}")
        endif()
    endmacro()
    get_WIN32_WINNT(ver)
    add_definitions(-D_WIN32_WINNT=${ver})
elseif(UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g3 -ggdb")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS}")
        message(STATUS "Debug build!")
    elseif(CMAKE_BUILD_TYPE MATCHES "Release")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS}")
        message(STATUS "Release build!")
    endif()
endif()

# date
find_package(date CONFIG REQUIRED)
#target_link_libraries(main PRIVATE date::date date::date-tz)

# magic-enum
find_package(magic_enum CONFIG REQUIRED)
#target_link_libraries(main PRIVATE magic_enum::magic_enum)

# protobuf
find_package(protobuf CONFIG REQUIRED)

# redis-plus-plus
find_package(redis++ CONFIG REQUIRED)
#target_link_libraries(main PRIVATE redis++::redis++_static)

# spdlog
find_package(spdlog CONFIG REQUIRED)
#target_link_libraries(main PRIVATE spdlog::spdlog spdlog::spdlog_header_only)

# tbb
#find_package(TBB CONFIG REQUIRED)
#target_link_libraries(main PRIVATE TBB::tbb TBB::tbbmalloc)

# boost
set (BOOST_MIN_VERSION "1.90.0")
find_package(Boost ${BOOST_MIN_VERSION} REQUIRED thread date_time regex filesystem json charconv)
message(STATUS "BOOST PATH: ${Boost_INCLUDE_DIRS}")

# openssl
find_package(OpenSSL REQUIRED)

# iguana 가져오기
FetchContent_Declare(
  iguana
  PREFIX "${CMAKE_CURRENT_BINARY_DIR}/iguana"
  GIT_REPOSITORY "https://github.com/qicosmos/iguana.git"
  GIT_PROGRESS TRUE
  GIT_SHALLOW  TRUE
  GIT_TAG "master")

FetchContent_GetProperties(iguana)
if(NOT iguana_POPULATED)
    FetchContent_Populate(iguana)
    add_subdirectory(${iguana_SOURCE_DIR} ${iguana_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${Boost_INCLUDE_DIRS}
)

if(MSVC)
    add_definitions(-DUNICODE -D_UNICODE)
    add_compile_options($<$<C_COMPILER_ID:MSVC>:/utf-8>)
    add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/utf-8>)

    add_compile_options(/W3;/WX;/sdl;/MP)

    add_compile_options(
        $<$<CONFIG:Debug>:/ZI>
        $<$<CONFIG:Debug>:/MTd>
        #$<$<CONFIG:Debug>:-fsanitize=address>
    )
    add_compile_options(
        $<$<CONFIG:RelWithDebInfo>:/Zi>
        $<$<CONFIG:RelWithDebInfo>:/MT>
        $<$<CONFIG:RelWithDebInfo>:/WX>
        $<$<CONFIG:RelWithDebInfo>:/GL>
    )
    add_compile_options(
        $<$<CONFIG:Release>:/Zi>
        $<$<CONFIG:Release>:/MT>
        $<$<CONFIG:Release>:/WX>
        $<$<CONFIG:Release>:/GL>
    )
    add_compile_options(/bigobj)
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/.editorconfig DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
elseif (UNIX)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_options(-fsanitize=address)
    endif()
endif()

# project
add_subdirectory(system)
add_subdirectory(utility)
add_subdirectory(network)
add_subdirectory(database)
add_subdirectory(protobuf)
add_subdirectory(chat_server)
add_subdirectory(dummy_client)

