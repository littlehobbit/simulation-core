cmake_minimum_required(VERSION 3.10)

project(simulation)

option(BUILD_TESTS ON)
option(RUN_CLANG_TIDY ON)
option(RUN_IWYU ON)

find_package(fmt REQUIRED)
find_package(Boost REQUIRED)
find_package(tinyxml2 REQUIRED)
find_package(ns3 REQUIRED)
find_package(CLI11 REQUIRED)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

add_executable(
  ${PROJECT_NAME}
  src/main.cpp
  src/app_config.cpp
)

add_library(
  ${PROJECT_NAME}_lib
  src/parser/parser.cpp
  src/parser/attribute_error.cpp
  src/model/model.cpp
  src/model/device.cpp
  src/model/node.cpp
  src/model/application.cpp
  src/model/channel.cpp
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
  ns3::libapplications
)

set(LIB_AS_NEEDED_PRE -Wl,--no-as-needed)
set(LIB_AS_NEEDED_POST -Wl,--as-needed)

target_link_libraries(
  ${PROJECT_NAME}_lib PUBLIC
  tinyxml2::tinyxml2
  fmt::fmt
  ${Boost_LIBRARIES}
)

target_link_libraries(
  ${PROJECT_NAME}
  ${PROJECT_NAME}_lib
  CLI11::CLI11

  # FIX: fix for loading static type information on start-up
  ${LIB_AS_NEEDED_PRE}
  ${NS3_LIBS}
  ${LIB_AS_NEEDED_POST}
)

include(clang-tidy)
include(iwyu)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
target_compile_features(${PROJECT_NAME}_lib PRIVATE cxx_std_17)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)
set_property(TARGET ${PROJECT_NAME}_lib PROPERTY CXX_STANDARD 17)


if (${BUILD_TESTS})
  enable_testing()
  add_subdirectory(tests)
endif() 