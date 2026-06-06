include(fetch_package)

fetch_package(NAME cpp-logger cpp_logger CPP_LOGGER Cpp_logger
              VERSION 1.1.1
              GIT https://github.com/JaeseungYeom/cpp-logger.git
              TAG cmake_fix
              #GIT https://github.com/hariharan-devarajan/cpp-logger.git
              #TAG b18d21b1ce67da49196a195d0cb496bd083cca38
              #SHALLOW
              DIAG
              VARS "CPP_LOGGER_ENABLE_TESTING OFF BOOL"
                   "CPP_LOGGER_LIBDIR_AS_LIB ${DYAD_LIBDIR_AS_LIB} BOOL")
