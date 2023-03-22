project(${PROJECT_NAME}_tests)

find_package(GTest REQUIRED)

include(GoogleTest)

add_executable(
  ${PROJECT_NAME}
  xml_parser_tests.cpp
  address_tests.cpp
  model_tests.cpp
  name_service_tests.cpp
  set_attribute_tests.cpp
)

target_include_directories(
  ${PROJECT_NAME} PRIVATE 
  ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(
  ${PROJECT_NAME}
  simulation_lib
  GTest::GTest
  GTest::Main
  gmock

  # FIX: fix for loading static type information on start-up
  ${LIB_AS_NEEDED_PRE}
  ${NS3_LIBS}
  ${LIB_AS_NEEDED_POST}
)

gtest_discover_tests(${PROJECT_NAME})

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)


