# ============================================================================
#  globalizer_install.cmake
#  ---------------------------------------------------------------------------
#  Установка заголовков, экспорт целей и генерация GlobalizerConfig.cmake
#  для подключения Globalizer из внешних проектов через find_package.
#  Поведение идентично прежнему монолиту.
# ============================================================================

function(globalizer_setup_install)
  include(GNUInstallDirs)

  # --- Публичные заголовки по модулям ---------------------------------------
  install(DIRECTORY
      ${GLOBALIZER_COMMON_SRC_DIR}/include/
      ${GLOBALIZER_PARAMETERS_DIR}/include/
      ${GLOBALIZER_evolvent_DIR}/include/
      ${GLOBALIZER_extended_DIR}/include/
      ${GLOBALIZER_output_system_DIR}/include/
      ${GLOBALIZER_problem_DIR}/include/
      ${GLOBALIZER_method_DIR}/include/
      ${GLOBALIZER_calculation_DIR}/include/
      ${GLOBALIZER_local_method_DIR}/include/
      DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
      FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp")

  # Если MPI выключен — ставим заглушку mpi.h.
  if(NOT GLOBALIZER_USE_MPI)
    install(DIRECTORY ${PROJECT_SOURCE_DIR}/lib/mpi_stub/include/
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
            FILES_MATCHING PATTERN "*.h")
  endif()

  # Делаем пути mpi_stub "экспорт-безопасными".
  if(TARGET mpi_stub)
    set_target_properties(mpi_stub PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES
      "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/lib/mpi_stub/include>;$<INSTALL_INTERFACE:include>")
  endif()

  # --- Экспорт целей ---------------------------------------------------------
  set(_install_targets ${GLOBALIZER_LIB_NAME})
  if(TARGET mpi_stub)
    list(APPEND _install_targets mpi_stub)
  endif()

  install(TARGETS ${_install_targets}
          EXPORT GlobalizerTargets
          LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
          ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
          RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

  install(EXPORT GlobalizerTargets
          FILE GlobalizerTargets.cmake
          NAMESPACE Globalizer::
          DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Globalizer)

  file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/GlobalizerConfig.cmake"
"include(CMakeFindDependencyMacro)\nfind_dependency(OpenMP)\ninclude(\"\${CMAKE_CURRENT_LIST_DIR}/GlobalizerTargets.cmake\")\n")

  install(FILES "${CMAKE_CURRENT_BINARY_DIR}/GlobalizerConfig.cmake"
          DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Globalizer)
endfunction()
