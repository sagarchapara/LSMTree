cmake_minimum_required(VERSION 3.10)

# Set the project name
project(LSMTreeProject)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add the source files
add_executable(LSMTree
    sstable.cpp
    lsmtree.cpp
    sstable.h
    lsmtree.h
)

# Include directories
target_include_directories(LSMTree PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Add clang-tidy for Google C++ coding standards
find_program(CLANG_TIDY "clang-tidy")
if(CLANG_TIDY)
    set_target_properties(LSMTree PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY};--checks=*,-llvm-header-guard,-readability-identifier-naming")
endif()

# Add cpplint for Google C++ coding standards
find_program(CPPLINT "cpplint")
if(CPPLINT)
    add_custom_target(cpplint
        COMMAND ${CPPLINT} --filter=-build/header_guard,-readability/identifier_naming ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/*.h
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Running cpplint..."
    )
endif()