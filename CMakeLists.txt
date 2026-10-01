cmake_minimum_required(VERSION 3.20)
project(nr_ofdm_reference LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(nr_ofdm STATIC cpp/src/ofdm.cpp)
target_include_directories(nr_ofdm PUBLIC cpp/include)
add_library(nr_q15 STATIC c/src/q15.c)
target_include_directories(nr_q15 PUBLIC c/include)

if(MSVC)
  target_compile_options(nr_ofdm PRIVATE /W4 /WX)
  target_compile_options(nr_q15 PRIVATE /W4 /WX)
else()
  target_compile_options(nr_ofdm PRIVATE -Wall -Wextra -Wpedantic -Werror)
  target_compile_options(nr_q15 PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

add_executable(nr_ofdm_frame cpp/app/frame.cpp)
target_link_libraries(nr_ofdm_frame PRIVATE nr_ofdm)
add_executable(nr_ofdm_tests cpp/tests/test_ofdm.cpp)
target_link_libraries(nr_ofdm_tests PRIVATE nr_ofdm)
add_executable(nr_q15_tests c/tests/test_q15.c)
target_link_libraries(nr_q15_tests PRIVATE nr_q15)

enable_testing()
add_test(NAME cpp_unit COMMAND nr_ofdm_tests)
add_test(NAME c_q15_unit COMMAND nr_q15_tests)
find_package(Python3 COMPONENTS Interpreter REQUIRED)
add_test(NAME python_cpp_parity
  COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/cpp/tests/test_python_parity.py $<TARGET_FILE:nr_ofdm_frame>)
set_tests_properties(python_cpp_parity PROPERTIES WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
