cmake_minimum_required(VERSION 3.14)

project(ice-lang VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

IF (CMAKE_BUILD_TYPE STREQUAL Debug)
    set(CMAKE_CXX_FLAGS "-g -Wall")
    ADD_DEFINITIONS(-D_DEBUG)

    file(GLOB SOURCES "src/*.cpp")
    add_executable(ice ${SOURCES})
ELSE()
    set(CMAKE_CXX_FLAGS "-O3 -Wall")

    add_library(tokenizer SHARED
        src/token.cpp
        src/tokenizer.cpp
    )

    add_library(parser SHARED
        src/parser.cpp
    )

    add_library(code SHARED
        src/code.cpp
        src/codegen.cpp
    )

    file(GLOB OBJECTS "src/*object.cpp")

    add_library(objects SHARED
        ${OBJECTS}
    )

    add_library(context SHARED
        src/context.cpp
        src/builtins.cpp
    )

    add_library(repl SHARED
        src/repl.cpp
    )

    install (
        TARGETS
            tokenizer
            parser
            code
            objects
            context
            repl
        DESTINATION lib
    )

    link_directories(${CMAKE_INSTALL_PREFIX}/lib)

    add_executable(ice src/main.cpp)
    target_link_libraries(ice
        PRIVATE
            tokenizer
            parser
            code
            objects
            context
            repl
    )

    set_target_properties(ice PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)
    install (TARGETS ice DESTINATION bin)
ENDIF()
