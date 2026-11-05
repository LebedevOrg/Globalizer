# ============================================================================
#  globalizer_ide.cmake
#  ---------------------------------------------------------------------------
#  Организация проектов и файлов для Visual Studio: раскладка 3rd-party и
#  multiple-precision целей по папкам решения + source_group для исходников.
#  Работает только под MSVC (как и в прежнем монолите).
# ============================================================================

function(globalizer_setup_ide_layout)
  if (NOT IS_MICROSOFT_COMPILER)
    return()
  endif()

  set_property(GLOBAL PROPERTY USE_FOLDERS ON)

  # --- Папка "3rd-party" -----------------------------------------------------
  set(_third_party "")
  if(GLOBALIZER_BUILD_TESTS)
    list(APPEND _third_party "gtest" "gmock" "gmock_main" "gtest_main")
  endif()
  if(NOT GLOBALIZER_USE_MPI)
    list(APPEND _third_party "mpi_stub")
  endif()
  if(GLOBALIZER_BUILD_GCGEN)
    list(APPEND _third_party "builder_sample")
  endif()
  if(_third_party)
    set_property(TARGET ${_third_party} PROPERTY FOLDER "3rd-party")
  endif()

  # --- Папка "multiple precision" --------------------------------------------
  set(_mp "")
  if(GLOBALIZER_USE_MP)
    list(APPEND _mp "bootstrap" "gen-bases" "gen-fac" "gen-fac-table" "gen-fib" "gen-fib-table"
      "gen-jacobitab" "gen-mp-bases" "gen-mpn-jacobitab" "gen-mpn-perfsqr" "gen-psqr" "gen-trialdivtab"
      "gen-trialdivtab-h" "libgmp" "libmpf" "libmpn" "libmpq" "libmpz" "libprintf" "librandom" "libscanf"
      "mpfr" "mpir")
  endif()
  if(_mp)
    set_property(TARGET ${_mp} PROPERTY FOLDER "multiple precision")
  endif()

  # --- source_group: заголовки ----------------------------------------------
  source_group("Header Files"                 FILES ${ROOT_HEADERS})
  source_group("Header Files\\Parameters"     FILES ${PARAMETERS_HEADERS})
  source_group("Header Files\\evolvent"       FILES ${evolvent_HEADERS})
  source_group("Header Files\\extended"       FILES ${extended_HEADERS})
  source_group("Header Files\\output_system"  FILES ${output_system_HEADERS})
  source_group("Header Files\\problem"        FILES ${problem_HEADERS} ${GLOBALIZER_BENCHMARKS_Interface_HEADERS})
  source_group("Header Files\\method"         FILES ${method_HEADERS})
  source_group("Header Files\\calculation"    FILES ${calculation_HEADERS})
  source_group("Header Files\\local_method"   FILES ${local_method_HEADERS})
  source_group("Header Files\\gcgen"          FILES ${GCGEN_HEADERS})

  # --- source_group: исходники ----------------------------------------------
  source_group("Source Files"                 FILES ${ROOT_SOURCES})
  source_group("Source Files\\evolvent"       FILES ${evolvent_SOURCES})
  source_group("Source Files\\extended"       FILES ${extended_SOURCES})
  source_group("Source Files\\output_system"  FILES ${output_system_SOURCES})
  source_group("Source Files\\problem"        FILES ${problem_SOURCES} ${GLOBALIZER_BENCHMARKS_Interface_SOURCES})
  source_group("Source Files\\method"         FILES ${method_SOURCES})
  source_group("Source Files\\calculation"    FILES ${calculation_SOURCES})
  source_group("Source Files\\local_method"   FILES ${local_method_SOURCES})
  source_group("Source Files\\parameters"     FILES ${PARAMETERS_SOURCES})
  source_group("Source Files\\gcgen"          FILES ${GCGEN_SOURCES})
endfunction()
