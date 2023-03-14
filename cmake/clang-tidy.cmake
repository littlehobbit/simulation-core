if(${RUN_CLANG_TIDY})
  find_program(
    CLANG_TIDY_EXE
    NAMES "clang-tidy"
    DOC "Path to clang-tidy executable"
  )

  if(NOT CLANG_TIDY_EXE)
    message(STATUS "clang-tidy not found.")
  else()
    # Подсмотерно в репозитории cmake
    # от файла .clang-tidy берётся sha1 и в качестве строки добавляется как target_compile_definitions для нужного таргета 
    # таким образом достигается полная пересборка таргета в случае изменения файла .clang-tidy
    #
    file(SHA1 ${CMAKE_SOURCE_DIR}/.clang-tidy clang_tidy_sha1)
    set(CLANG_TIDY_DEFINITIONS "CLANG_TIDY_SHA1=${clang_tidy_sha1}")
    unset(clang_tidy_sha1)

    message(STATUS "clang-tidy found: ${CLANG_TIDY_EXE}")
    set(DO_CLANG_TIDY "${CLANG_TIDY_EXE}" "-p=${CMAKE_BINARY_DIR}" "-config-file=${CMAKE_SOURCE_DIR}/.clang-tidy")
  endif()

  if(CLANG_TIDY_EXE)
    set_target_properties(
      ${PROJECT_NAME} PROPERTIES
      CXX_CLANG_TIDY "${DO_CLANG_TIDY}"
    )

    set_target_properties(
      ${PROJECT_NAME}_lib PROPERTIES
      CXX_CLANG_TIDY "${DO_CLANG_TIDY}"
    )

    target_compile_definitions(${PROJECT_NAME} PUBLIC ${CLANG_TIDY_DEFINITIONS})
    target_compile_definitions(${PROJECT_NAME}_lib PUBLIC ${CLANG_TIDY_DEFINITIONS})

    configure_file(${CMAKE_SOURCE_DIR}/.clang-tidy ${CMAKE_SOURCE_DIR}/.clang-tidy COPYONLY)
  endif()
endif()