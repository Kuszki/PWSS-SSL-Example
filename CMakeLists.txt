cmake_minimum_required(VERSION 3.5)

project(PWSS-SSL-Example LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})
link_directories(${OPENSSL_LIBRARY_DIR})


add_library(SSL_wrapper SHARED
	wrapper.hpp wrapper.cpp
	server.hpp server.cpp
	client.hpp client.cpp)

list(APPEND LIB_LIST SSL_wrapper ${OPENSSL_LIBRARIES})

target_compile_definitions(SSL_wrapper PUBLIC SSL_wrapper)

add_executable(SSL_serwer srvmain.cpp)
add_executable(SSL_klient climain.cpp)

target_link_libraries(SSL_serwer ${LIB_LIST})
target_link_libraries(SSL_klient ${LIB_LIST})
