include(fetch_package)

fetch_package(NAME Catch2 catch2
              VERSION 3.15.0
              GIT https://github.com/catchorg/Catch2.git
              TAG v3.15.0
              REQUIRED
              DIAG
              VARS "BUILD_TESTING OFF BOOL"
                   "CMAKE_INSTALL_LIBDIR ${CMAKE_INSTALL_LIBDIR} PATH")
