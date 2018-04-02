cmake_minimum_required(VERSION 2.8.9)
project(relz_fs)
set(CMAKE_BUILD_TYPE Release)

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -O3")
elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -Wall -O3")
else()
    message(FATAL_ERROR "Compiler ${CMAKE_CXX_COMPILER} has no C++11 support.")
endif()

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include_directories(${CMAKE_SOURCE_DIR}/include)
file(GLOB SOURCES "${CMAKE_SOURCE_DIR}/src/*.cpp")

add_executable(compress "${CMAKE_SOURCE_DIR}/programs/compress.cpp")
add_executable(decompress "${CMAKE_SOURCE_DIR}/programs/decompress.cpp")
add_executable(daemon_fuse_hybrid "${CMAKE_SOURCE_DIR}/programs/daemon_fuse_hybrid.cpp")

add_library(relz_fs SHARED ${SOURCES})
link_directories(${CMAKE_BINARY_DIR})

target_link_libraries(compress relz_fs pthread)
target_link_libraries(decompress relz_fs pthread)
target_link_libraries(daemon_fuse_hybrid relz_fs pthread)

if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set (CMAKE_INSTALL_PREFIX "/usr/" CACHE PATH "default install path" FORCE )
endif()

install(TARGETS relz_fs DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
install(TARGETS compress DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/ DESTINATION ${CMAKE_INSTALL_PREFIX}/include/relz_fs FILES_MATCHING PATTERN "*.h")

