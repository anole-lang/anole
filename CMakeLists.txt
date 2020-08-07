cmake_minimum_required (VERSION 3.14)

set (CMAKE_CXX_COMPILER /usr/bin/clang++)

project (anole VERSION 0.0.17)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED True)

set (CMAKE_EXE_LINKER_FLAGS "-rdynamic")
set (CMAKE_SHARED_LINKER_FLAGS "-rdynamic")

IF (CMAKE_BUILD_TYPE STREQUAL Debug)
    set (CMAKE_CXX_FLAGS "-g -Wall")
    ADD_DEFINITIONS (-D_DEBUG)

    file (GLOB SOURCES "src/*.cpp")
    add_executable (anole ${SOURCES})

    target_link_libraries (anole PRIVATE
        dl
        stdc++fs
        readline
    )
    install (TARGETS anole DESTINATION bin)
ELSEIF (CMAKE_BUILD_TYPE STREQUAL Gprof)
    set (CMAKE_CXX_FLAGS "-pg -O3 -Wall")

    file (GLOB SOURCES "src/*.cpp")
    add_executable (anole ${SOURCES})

    target_link_libraries (anole PRIVATE
        dl
        stdc++fs
        readline
    )
    install (TARGETS anole DESTINATION bin)
ELSEIF (CMAKE_BUILD_TYPE STREQUAL Dynamic)
    set (CMAKE_CXX_FLAGS "-O3 -Wall")

    add_library (error SHARED
        src/error.cpp
    )

    add_library (tokenizer SHARED
        src/token.cpp
        src/tokenizer.cpp
    )

    add_library (parser SHARED
        src/parser.cpp
    )

    add_library (code SHARED
        src/code.cpp
        src/codegen.cpp
    )

    file (GLOB OBJECTS "src/*object.cpp")

    add_library (objects SHARED
        ${OBJECTS}
    )

    add_library (context SHARED
        src/context.cpp
        src/builtins.cpp
    )

    add_library (repl SHARED
        src/repl.cpp
    )

    install (
        TARGETS
            error
            tokenizer
            parser
            code
            objects
            context
            repl
        DESTINATION "lib/anole"
    )

    link_directories (${CMAKE_INSTALL_PREFIX}/lib/anole)

    add_executable (anole src/main.cpp)
    target_link_libraries (anole PRIVATE
        error
        tokenizer
        parser
        code
        objects
        context
        repl
        dl
        stdc++fs
        readline
    )

    set_target_properties (anole PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)
    install (TARGETS anole DESTINATION bin)
ELSE ()
    set (CMAKE_CXX_FLAGS "-O3 -Wall")

    file (GLOB SOURCES "src/*.cpp")
    add_executable (anole ${SOURCES})
    target_link_libraries (anole PRIVATE
        dl
        stdc++fs
        readline
    )
    install (TARGETS anole
        DESTINATION bin
    )
ENDIF ()

# install headers of anole
file (GLOB HEADERS "src/*.hpp")

install (FILES ${HEADERS}
    DESTINATION "include/anole"
)

#install libs of anole
#install lib file
add_library (fileobject SHARED
    lib/file/fileobject.cpp
)

install (FILES "lib/file/__init__.anole"
    DESTINATION "lib/anole/file"
)

install (TARGETS fileobject
    DESTINATION "lib/anole/file"
)

#install lib env
add_library (env SHARED
    lib/env/env.cpp
)

install (FILES "lib/env/__init__.anole"
    DESTINATION "lib/anole/env"
)

install (TARGETS env
    DESTINATION "lib/anole/env"
)

#install lib os
install (FILES "lib/os/__init__.anole"
    DESTINATION "lib/anole/os"
)

#install lib path of lib os
add_library ("path" SHARED
    "lib/os/path/path.cpp"
)

install (FILES "lib/os/path/__init__.anole"
    DESTINATION "lib/anole/os/path"
)

install (TARGETS "path"
    DESTINATION "lib/anole/os/path"
)

#install lib read_dir of lib os
add_library ("read_dir" SHARED
    "lib/os/read_dir/read_dir.cpp"
)

install (FILES "lib/os/read_dir/__init__.anole"
    DESTINATION "lib/anole/os/read_dir"
)

install (TARGETS "read_dir"
    DESTINATION "lib/anole/os/read_dir"
)

#install lib coroutine
install (FILES "lib/coroutine/__init__.anole"
    DESTINATION "lib/anole/coroutine"
)
