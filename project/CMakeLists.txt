add_executable(calculator main.cpp)

target_include_directories(calculator PRIVATE
	${CMAKE_SOURCE_DIR}/math_lib
)

install(TARGETS calculator DESTINATION bin)
