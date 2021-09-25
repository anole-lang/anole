cmake_minimum_required (VERSION 3.14)

set (CMAKE_CXX_COMPILER /usr/bin/clang++)

project (anole)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED True)

set (CMAKE_EXE_LINKER_FLAGS "-rdynamic")
set (CMAKE_SHARED_LINKER_FLAGS "-rdynamic")

file (GLOB SRC_MAIN "anole/*.cpp")
file (GLOB SRC_OBJECTS "anole/objects/*.cpp")
file (GLOB SRC_RUNTIME "anole/runtime/*.cpp")
file (GLOB SRC_COMPILER "anole/compiler/*.cpp")

IF (CMAKE_BUILD_TYPE STREQUAL Debug)
    set (CMAKE_CXX_FLAGS "-g -Wall")
    ADD_DEFINITIONS (-D_DEBUG)

    add_executable (anole ${SRC_MAIN} ${SRC_OBJECTS} ${SRC_RUNTIME} ${SRC_COMPILER})

    target_link_libraries (anole PRIVATE
        dl
        stdc++fs
        readline
    )
    install (TARGETS anole DESTINATION bin)

ELSEIF (CMAKE_BUILD_TYPE STREQUAL Gprof)
    set (CMAKE_CXX_FLAGS "-pg -O3 -Wall")

    add_executable (anole ${SRC_MAIN} ${SRC_OBJECTS} ${SRC_RUNTIME} ${SRC_COMPILER})

    target_link_libraries (anole PRIVATE
        dl
        stdc++fs
        readline
    )
    install (TARGETS anole DESTINATION bin)

ELSEIF (CMAKE_BUILD_TYPE STREQUAL Dynamic)
    set (CMAKE_CXX_FLAGS "-O3 -Wall")

    add_library (objects SHARED
        ${SRC_OBJECTS}
    )

    add_library (runtime SHARED
        ${SRC_RUNTIME}
    )

    add_library (compiler SHARED
        ${SRC_COMPILER}
    )

    install (
        TARGETS
            objects
            runtime
            compiler
        DESTINATION "lib/anole"
    )

    link_directories (${CMAKE_INSTALL_PREFIX}/lib/anole)

    add_executable (anole ${SRC_MAIN})
    target_link_libraries (anole PRIVATE
        objects
        runtime
        compiler

        dl
        stdc++fs
        readline
    )

    set_target_properties (anole PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)
    install (TARGETS anole DESTINATION bin)

ELSE ()
    set (CMAKE_CXX_FLAGS "-O3 -Wall")

    add_executable (anole ${SRC_MAIN} ${SRC_OBJECTS} ${SRC_RUNTIME} ${SRC_COMPILER})
    target_link_libraries (anole PRIVATE
        dl
        stdc++fs
        readline
    )
    install (TARGETS anole
        DESTINATION bin
    )

ENDIF ()

# --- install lib ---

# install header `anole.hpp`
install (FILES "anole/anole.hpp"
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

# --- anole test ---
enable_testing()

add_executable(
    anole_test
    test/test.cpp
    anole/error.cpp
    ${SRC_OBJECTS} ${SRC_RUNTIME} ${SRC_COMPILER}
)
target_link_libraries(
    anole_test
    dl
    stdc++fs
    readline

    pthread
    gtest
)

include(GoogleTest)
gtest_discover_tests(anole_test)
