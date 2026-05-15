cmake_minimum_required(VERSION 3.15)
project(calculator)

option(USE_CLANG_FORMAT "Enable clang-format" OFF)
option(USE_CLANG_TIDY "Enable clang-tidy" OFF)

if(USE_CLANG_FORMAT)
    find_program(CLANG_FORMAT clang-format-21 REQUIRED)
    add_custom_target(clang-format-fix
        COMMAND python3 /usr/local/bin/run-clang-format.py 
	--clang-format-executable clang-format-21
	-i -r ${CMAKE_SOURCE_DIR}/project
    )
endif()

if(USE_CLANG_TIDY)
    find_program(CLANG_TIDY clang-tidy-21)
    set(CMAKE_CXX_CLANG_TIDY ${CLANG_TIDY})
endif()

add_subdirectory(project)
