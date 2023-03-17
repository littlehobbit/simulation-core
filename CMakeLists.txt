cmake_minimum_required(VERSION 3.10)

project(simulation)

option(BUILD_TESTS ON)
option(RUN_CLANG_TIDY ON)
option(RUN_IWYU ON)

# TODO: attach
# - ns-3 enviroment

find_package(fmt REQUIRED)
find_package(Boost REQUIRED)
find_package(tinyxml2 REQUIRED)
find_package(ns3 REQUIRED)
find_package(CLI11 REQUIRED)

add_executable(
  ${PROJECT_NAME}
  src/main.cpp
  src/app_config.cpp
)

add_library(
  ${PROJECT_NAME}_lib
  src/parser/parser.cpp
  src/parser/attribute_error.cpp
)

target_include_directories(
  ${PROJECT_NAME}_lib PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/src
)

set(NS3_LIBS 
  ns3::libcore
  ns3::libnetwork
  ns3::libinternet
  ns3::libstats
  ns3::libcsma
  ns3::libpoint-to-point
)

target_link_libraries(
  ${PROJECT_NAME}_lib PUBLIC
  tinyxml2::tinyxml2
  fmt::fmt
  ${Boost_LIBRARIES}
  ${NS3_LIBS}
)

target_link_libraries(
  ${PROJECT_NAME}
  ${PROJECT_NAME}_lib
  CLI11::CLI11
)

include(cmake/clang-tidy.cmake)
include(cmake/iwyu.cmake)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
target_compile_features(${PROJECT_NAME}_lib PRIVATE cxx_std_17)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)
set_property(TARGET ${PROJECT_NAME}_lib PROPERTY CXX_STANDARD 17)


if (${BUILD_TESTS})
  enable_testing()
  add_subdirectory(tests)
endif() 