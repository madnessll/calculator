add_executable(calculator main.cpp)

target_include_directories(calculator PRIVATE 
    ${math_lib_SOURCE_DIR}
)

install(TARGETS calculator DESTINATION bin)
