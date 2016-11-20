cmake_minimum_required(VERSION 3.0.0)

set(PROJECT_NAME csrv)
project(${PROJECT_NAME} C CXX)

find_package(Threads REQUIRED)

if(${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
  add_definitions(-Wall -std=c++11) #-pthread)
endif()

MESSAGE(${PROJECT_SOURCE_DIR})

FILE( GLOB_RECURSE PROJ_SRC ${PROJECT_SOURCE_DIR}/src/**/*.cc )

add_executable(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/main.cc ${PROJ_SRC})
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(csrv ${CMAKE_THREAD_LIBS_INIT})
