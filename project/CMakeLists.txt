add_executable(calculator main.cpp logger.cpp)

target_include_directories(calculator PRIVATE 
    ${math_lib_SOURCE_DIR}
)

target_link_libraries(calculator PRIVATE 
    nlohmann_json::nlohmann_json
    spdlog::spdlog
)

install(TARGETS calculator DESTINATION bin)
