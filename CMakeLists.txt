cmake_minimum_required(VERSION 3.5)

project(luminar LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(src)

file(GLOB_RECURSE LUMINAR_SOURCES "src/*.cpp")
file(GLOB_RECURSE TEST_SOURCES "test/*.cpp")

add_library(libluminar STATIC ${LUMINAR_SOURCES})

add_executable(luminar src/main.cpp)
target_link_libraries(luminar libluminar -L/usr/lib/gcc/x86_64-linux-gnu/13/ -lgccjit)

include(GNUInstallDirs)
install(TARGETS luminar
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

enable_testing()

find_package(GTest REQUIRED)

add_executable(runUnitTests ${TEST_SOURCES})
target_link_libraries(runUnitTests GTest::gtest GTest::gtest_main libluminar)

add_test(NAME runUnitTests COMMAND runUnitTests)


# Install the documentation files
install(FILES
    doc/readme.md
    doc/issues.md
    doc/features.md
    doc/design.md
    doc/implementation.md
    doc/zen.md
    DESTINATION ${CMAKE_INSTALL_DOCDIR}
)
