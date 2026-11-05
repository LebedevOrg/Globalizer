# ============================================================================
#  globalizer_compiler.cmake
#  ---------------------------------------------------------------------------
#  Настройка флагов компилятора, AddressSanitizer и ABI-определений.
#  Функции вызываются из корневого CMakeLists ПОСЛЕ GLOBALIZER_initialize()
#  (нужны IS_GNU_COMPILER / IS_MICROSOFT_COMPILER / GLOBALIZER_TARGET_ARCH).
# ============================================================================

# ---------------------------------------------------------------------------
#  ABI-макросы: определяют раскладку структур (Trial, Extended, Parameters...).
#  Должны совпадать у библиотеки и у всех потребителей. Экспортируются через
#  target_compile_definitions(... PUBLIC ...), поэтому приложению задавать их
#  вручную НЕ нужно. Результат кладётся в переменную GLOBALIZER_ABI_DEFINITIONS
#  в родительском (directory) scope.
# ---------------------------------------------------------------------------
function(globalizer_init_abi_definitions)
  set(GLOBALIZER_MAX_DIMENSION "10" CACHE STRING "Maximum problem dimension (ABI)")
  set(GLOBALIZER_MAX_Number_Of_Function "20" CACHE STRING "Maximum number of functions (ABI)")

  set(_defs
      GLOBALIZER_MAX_DIMENSION=${GLOBALIZER_MAX_DIMENSION}
      GLOBALIZER_MAX_Number_Of_Function=${GLOBALIZER_MAX_Number_Of_Function})

  set(GLOBALIZER_ABI_DEFINITIONS "${_defs}" PARENT_SCOPE)
  message(STATUS "Globalizer ABI definitions: ${_defs}")
endfunction()

# ---------------------------------------------------------------------------
#  Флаги компилятора по платформе/компилятору. Поведение идентично прежнему
#  монолиту: GNU (-O3 + ccache + GMPConfig), MSVC (/EHsc, W3->W4, /Ox...),
#  прочее (OpenMP-флаги).
# ---------------------------------------------------------------------------
macro(globalizer_setup_compiler_flags)
  if (IS_GNU_COMPILER)
    include(${CMAKE_SOURCE_DIR}/cmake/GMPConfig.cmake)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
    find_program(CCACHE_FOUND ccache)
    if(CCACHE_FOUND)
      set(CMAKE_CXX_COMPILER_LAUNCHER ccache)
    endif()
  elseif (IS_MICROSOFT_COMPILER)
    add_definitions(/EHsc -D_CRT_SECURE_NO_WARNINGS -DNOMINMAX)
    replace_compiler_flag("W3" "W4")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Ox /wd4100")
    if (GLOBALIZER_TARGET_ARCH MATCHES "x86")
      set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /arch:SSE2")
    endif()
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /wd4100")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ignore:4099")
  else()
    set(OpenMP_C_FLAGS   "-fopenmp")
    set(OpenMP_CXX_FLAGS "-fopenmp")
  endif()
endmacro()

# ---------------------------------------------------------------------------
#  AddressSanitizer (диагностика). Включается опцией GLOBALIZER_USE_ASAN.
# ---------------------------------------------------------------------------
macro(globalizer_setup_asan)
  if (GLOBALIZER_USE_ASAN)
    message(STATUS "AddressSanitizer ENABLED")
    if (IS_MICROSOFT_COMPILER)
      # MSVC / clang-cl: с ASan несовместимы Edit&Continue и инкрем. линковка
      add_compile_options(/fsanitize=address /Zi)
      string(REPLACE "/RTC1" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
      string(REPLACE "/INCREMENTAL" "/INCREMENTAL:NO" CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")
    else()
      add_compile_options(-fsanitize=address -fno-omit-frame-pointer -g)
      add_link_options(-fsanitize=address)
    endif()
  endif()
endmacro()

# ---------------------------------------------------------------------------
#  Тип сборки по умолчанию: если не из списка конфигураций — Release.
# ---------------------------------------------------------------------------
macro(globalizer_normalize_build_type)
  if(NOT (CMAKE_BUILD_TYPE IN_LIST CMAKE_CONFIGURATION_TYPES))
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "build type" FORCE)
    message(STATUS "Unknown build type has been changed to " ${CMAKE_BUILD_TYPE})
  endif()
endmacro()
