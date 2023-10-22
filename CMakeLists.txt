cmake_minimum_required(VERSION 3.10)

set(VERSION "0.0.0" CACHE STRING "Project version")
project(simulation VERSION ${VERSION} LANGUAGES CXX)

option(BUILD_TESTS OFF)
option(BUILD_PACKAGE OFF)
option(RUN_CLANG_TIDY OFF)
option(RUN_IWYU OFF)
option(ENABLE_COVERAGE OFF)

find_package(fmt REQUIRED)
find_package(Boost REQUIRED)
find_package(tinyxml2 REQUIRED)
find_package(CLI11 REQUIRED)

find_package(ns3 REQUIRED)


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
  
add_executable(
  ${PROJECT_NAME}
  src/main.cpp
  src/app_config.cpp
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

include(cmake/clang-tidy.cmake)
include(cmake/iwyu.cmake)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
target_compile_features(${PROJECT_NAME}_lib PRIVATE cxx_std_17)

set_target_properties(
  ${PROJECT_NAME} ${PROJECT_NAME}_lib
  PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED TRUE
)

if (${BUILD_TESTS})
  enable_testing()
  add_subdirectory(tests)

  if (${ENABLE_COVERAGE})
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/code_coverage.cmake)

    append_coverage_compiler_flags()
    append_coverage_compiler_flags_to_target(${PROJECT_NAME}_lib)

    set(
      COVERAGE_EXCLUDES
      "${PROJECT_SOURCE_DIR}/src/main.cpp"
      "${PROJECT_SOURCE_DIR}/src/app_config.cpp"
    )

    setup_target_for_coverage_gcovr_html(
      NAME gcovr_html
      EXECUTABLE ctest
      DEPENDENCIES ${PROJECT_NAME}_lib ${PROJECT_NAME}_tests
      BASE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/src"
    )

    setup_target_for_coverage_gcovr_xml(
      NAME gcovr
      EXECUTABLE ctest
      DEPENDENCIES ${PROJECT_NAME}_lib ${PROJECT_NAME}_tests
      BASE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/src"
    )
  endif()
endif() 

include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)

if (${BUILD_PACKAGE})
  set(CPACK_GENERATOR DEB)
  set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
  set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
  set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
  set(CPACK_PACKAGE_CONTACT gataullinruslandev@gmain.com)
  include(CPack)
endif()


if (${RUN_CLANG_TIDY})
  include(cmake/clang-tidy.cmake)
  setup_clang_tidy(${PROJECT_NAME} PUBLIC)
  setup_clang_tidy(${PROJECT_NAME}_lib INTERFACE)
endif()

if (${RUN_IWYU})
  include(cmake/iwyu.cmake)
  setup_iwyu_globally()
endif()