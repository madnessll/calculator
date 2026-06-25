include(FetchContent)

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.tar.gz
)
FetchContent_MakeAvailable(googletest)

add_executable(tests test_math.cpp)

target_include_directories(tests PRIVATE
    ${math_lib_SOURCE_DIR}
)

target_link_libraries(tests PRIVATE GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(tests)

find_program(VALGRIND valgrind)
if(VALGRIND)
    add_custom_target(valgrind_tests
        COMMAND ${VALGRIND} --leak-check=full --error-exitcode=1
        $<TARGET_FILE:tests>
        DEPENDS tests
    )
endif()