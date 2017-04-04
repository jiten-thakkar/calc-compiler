# adapted a bit from http://llvm.org/docs/CMake.html#embedding-llvm-in-your-project

cmake_minimum_required(VERSION 3.2)
project(IntervalBitwise)

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
#find_library (LIBSTDCXX NAMES stdc++)
#add_compile_options(-stdlib=libstdc++)

#set(LLVM_LINK_COMPONENTS
#  Core
#  ExecutionEngine
#  Interpreter
#  MC
#  Support
#  nativecodegen
#  )



set(CMAKE_CXX_FLAGS "-std=c++11 -fno-rtti -Wall -D__STDC_CONSTANT_MACROS -D__STDC_LIMIT_MACROS")

include_directories(${LLVM_INCLUDE_DIRS})
#add_definitions(${LLVM_DEFINITIONS})

add_executable(calcc calcc.cpp)

add_library(utils STATIC
utils.h
utils.cpp)

add_executable(calcc_basicjit calcc_basicjit.cpp)
add_executable(calcc_basicjit1 calcc_basicjit1.cpp)
add_executable(calcc_basicjit2 calcc_basicjit2.cpp)

llvm_map_components_to_libnames(llvm_libs support core irreader ExecutionEngine Interpreter MC nativecodegen mcjit native passes)

#target_link_libraries(calcc  ${LIBSTDCXX} ${llvm_libs})
target_link_libraries(calcc  ${llvm_libs})
target_link_libraries(calcc_basicjit  ${llvm_libs} utils)
target_link_libraries(calcc_basicjit1  ${llvm_libs} utils)
target_link_libraries(calcc_basicjit2  ${llvm_libs}  utils)

add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/auto-tests
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/auto-tests ${CMAKE_BINARY_DIR}/auto-tests
        )
add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/driver.c
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/driver.c ${CMAKE_BINARY_DIR}
        )

add_custom_target(check-all
        COMMAND ${CMAKE_COMMAND} -E env CALCC=${CMAKE_BINARY_DIR}/calcc PATH=${LLVM_TOOLS_BINARY_DIR}:$ENV{PATH} ${CMAKE_SOURCE_DIR}/run_tests.pl *.calc
        DEPENDS calcc ${CMAKE_BINARY_DIR}/auto-tests ${CMAKE_BINARY_DIR}/driver.c
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/auto-tests
        )
