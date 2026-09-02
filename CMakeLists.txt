cmake_minimum_required(VERSION 3.15)
project(calculator)

include(FetchContent)

FetchContent_Declare(
    math_lib
    GIT_REPOSITORY https://github.com/madnessll/math_lib.git
    GIT_TAG main
)

FetchContent_Declare(
    nlohmann_json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
)

FetchContent_MakeAvailable(math_lib nlohmann_json)

FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.13.0
)
FetchContent_MakeAvailable(spdlog)

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

add_subdirectory(tests)
add_subdirectory(project)

install(FILES config/calculator.service
    DESTINATION /etc/systemd/system
)

set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_NAME "calculator")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_PACKAGE_DESCRIPTION "Calculator service")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "iwillwin")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libpq5, libboost-system1.74.0")
include(CPack)