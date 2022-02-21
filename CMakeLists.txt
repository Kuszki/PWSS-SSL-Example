cmake_minimum_required(VERSION 3.5)

project(PWSS-SSL-Example LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(GEN_INC ${PROJECT_BINARY_DIR}/include)

find_package(Git)

find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})
link_directories(${OPENSSL_LIBRARY_DIR})

find_package(Boost COMPONENTS program_options REQUIRED)
include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIR})

execute_process(COMMAND
	${GIT_EXECUTABLE} rev-parse HEAD
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
	OUTPUT_VARIABLE GIT_HASH_LONG
	ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

execute_process(COMMAND
	${GIT_EXECUTABLE} rev-parse --short HEAD
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
	OUTPUT_VARIABLE GIT_HASH_SHORT
	ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

configure_file(version.hpp.in ${GEN_INC}/version.hpp)

add_library(SSL_wrapper SHARED
	${GEN_INC}/version.hpp
	wrapper.hpp wrapper.cpp
	server.hpp server.cpp
	client.hpp client.cpp)

list(APPEND LIB_LIST SSL_wrapper
	${OPENSSL_LIBRARIES}
	${Boost_LIBRARIES})

target_include_directories(SSL_wrapper PUBLIC ${GEN_INC}/include)
target_compile_definitions(SSL_wrapper PUBLIC SSL_wrapper)

add_executable(SSL_serwer
	srvmain.cpp srvmain.hpp
	options.hpp options.cpp)

add_executable(SSL_klient
	climain.cpp climain.hpp
	options.hpp options.cpp)

target_link_libraries(SSL_serwer ${LIB_LIST})
target_link_libraries(SSL_klient ${LIB_LIST})
