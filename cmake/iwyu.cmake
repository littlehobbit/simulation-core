if (${RUN_IWYU})
  find_program(IWYU_EXE 
    NAMES include-what-you-use iwyu
  )

  if(NOT IWYU_EXE)
    message(STATUS "include-what-you-use not found.")
  else()
    message(STATUS "include-what-you-use found: ${IWYU_EXE}")
    set(IWYU_EXE "${IWYU_EXE}" "-Xiwyu" "--mapping_file=${CMAKE_SOURCE_DIR}/.iwyu.imp" 
                               "-Xiwyu" "--verbose=1"
                               "-Xiwyu" "--cxx17ns"
    )
    
    set_property(
      TARGET ${PROJECT_NAME} 
      PROPERTY CXX_INCLUDE_WHAT_YOU_USE "${IWYU_EXE}"
    )
    
    set_property(
      TARGET ${PROJECT_NAME}_lib
      PROPERTY CXX_INCLUDE_WHAT_YOU_USE "${IWYU_EXE}"
    )
  endif()
endif()