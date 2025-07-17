function (setup_mpir arg_libdir arg_includedir arg_libname)
  get_platform_depended_library_path(LIB_PATH_PART)
  if (WIN32)
    set(${arg_libname} mpir PARENT_SCOPE)
  elseif(UNIX)
    set(${arg_libname} gmp PARENT_SCOPE)
  endif()
  set(${arg_includedir} ${PROJECT_SOURCE_DIR}/lib/mpir/include PARENT_SCOPE)
  set(${arg_libdir} ${PROJECT_SOURCE_DIR}/lib/mpir/${LIB_PATH_PART} PARENT_SCOPE)
endfunction (setup_mpir)


function (setup_mpfr arg_libdir arg_includedir arg_libname)
  get_platform_depended_library_path(LIB_PATH_PART)
  set(${arg_libname} mpfr PARENT_SCOPE)
  set(${arg_includedir} ${PROJECT_SOURCE_DIR}/lib/mpfr/include PARENT_SCOPE)
  set(${arg_libdir} ${PROJECT_SOURCE_DIR}/lib/mpfr/${LIB_PATH_PART} PARENT_SCOPE)
endfunction (setup_mpfr)

function (setup_mpreal arg_include)
  set(${arg_include} ${PROJECT_SOURCE_DIR}/lib/mpreal/include PARENT_SCOPE)
endfunction (setup_mpreal)


