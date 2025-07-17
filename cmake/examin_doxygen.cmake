function (init_doxygen)
  find_package(Doxygen)
  if(DOXYGEN_FOUND)
    set(DOXY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/doc)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
    add_custom_target(doc
    ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Generating API documentation with Doxygen" VERBATIM)
  endif(DOXYGEN_FOUND)
endfunction (init_doxygen)
