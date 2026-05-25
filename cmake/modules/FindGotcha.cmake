include(fetch_package)

fetch_package(NAME gotcha GOTCHA Gotcha
              VERSION 2.4.2
              GIT https://github.com/JaeseungYeom/GOTCHA.git
              TAG rm_empty_dir
              #GIT https://github.com/LLNL/GOTCHA.git
              #TAG 56def6fd26e436f01fb7831f784d0f9b1347904b
              REQUIRED
              DIAG
              VARS "GOTCHA_ENABLE_TESTING OFF BOOL"
                   "CMAKE_INSTALL_LIBDIR ${CMAKE_INSTALL_LIBDIR} PATH")

get_target_property(_gotcha_imported gotcha IMPORTED)
if (NOT _gotcha_imported OR _gotcha_imported MATCHES "NOTFOUND")
    # GOTCHA uses GNU-specific extensions. So it needs to be
    # compiled with -std=gnu11 instead of -std=c11.
    set_target_properties(gotcha PROPERTIES C_EXTENSIONS ON)
    # Setting gotcha include dirs
    target_include_directories(gotcha PUBLIC
        $<BUILD_INTERFACE:${gotcha_BINARY_DIR}/include>
        $<BUILD_INTERFACE:${gotcha_SOURCE_DIR}/include>
    )
endif ()
