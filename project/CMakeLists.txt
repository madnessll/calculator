find_package(PostgreSQL REQUIRED)

add_executable(calculator main.cpp logger.cpp)

target_include_directories(calculator PRIVATE 
    ${math_lib_SOURCE_DIR}
    /usr/include/postgresql
)


target_link_libraries(calculator PRIVATE 
    nlohmann_json::nlohmann_json
    spdlog::spdlog
    PostgreSQL::PostgreSQL
)

target_compile_features(calculator PRIVATE cxx_std_20)

install(TARGETS calculator DESTINATION bin)

if(USE_CLANG_TIDY)
    find_program(CLANG_TIDY clang-tidy-21)
    set_target_properties(calculator PROPERTIES CXX_CLANG_TIDY ${CLANG_TIDY})
endif()