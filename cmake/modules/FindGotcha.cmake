#.rst:
# FindGotcha
# -----------
#
# Find gotcha headers and libraries.
#
# ::
#
#   gotcha_FOUND          - True if gotcha found.
#   gotcha_INCLUDE_DIRS   - Where to find gotcha.h.
#   gotcha_LIBRARIES      - List of libraries when using gotcha.

#cmake_minimum_required(VERSION 3.0)

foreach (_gotcha_hint "$ENV{gotcha_DIR}" "$ENV{GOTCHA_DIR}"
                      "$ENV{gotcha_PATH}" "$ENV{GOTCHA_PATH}")
  if (_gotcha_hint)
    foreach (_suffix
             "lib/cmake/gotcha"   "lib/cmake/GOTCHA"
             "lib64/cmake/gotcha" "lib64/cmake/GOTCHA"
             "lib"                "lib64"
    )
        list(APPEND CMAKE_PREFIX_PATH "${_gotcha_hint}/${_suffix}")
    endforeach ()
    # Also add the hint root itself
    list(APPEND CMAKE_PREFIX_PATH "${_gotcha_hint}")
  endif ()
endforeach ()

set(gotcha_FOUND FALSE)

if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.14")
  message(STATUS "Gotcha: fetching if not found") 
  include(FetchContent)
  FetchContent_Declare(
    gotcha
    GIT_REPOSITORY https://github.com/llnl/GOTCHA.git
    GIT_TAG        1.0.8
    PATCH_COMMAND  sed -i "s/CMAKE_SOURCE_DIR/CMAKE_CURRENT_SOURCE_DIR/g" CMakeLists.txt &&
                   sed -i "s|PATH_VARS gotcha_INSTALL_INCLUDE_DIR PATH_VARS gotcha_INSTALL_LIBRARY_DIR|PATH_VARS gotcha_INSTALL_INCLUDE_DIR gotcha_INSTALL_LIBRARY_DIR|g" CMakeLists.txt &&
                   sed -i "s|add_subdirectory(example)||g" src/CMakeLists.txt
    FIND_PACKAGE_ARGS NAMES gotcha GOTCHA
  )
  set(GOTCHA_ENABLE_TESTS OFF CACHE BOOL "" FORCE)
  set(CMAKE_WARN_DEPRECATED OFF CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(gotcha)
  set(CMAKE_WARN_DEPRECATED ON CACHE BOOL "" FORCE)

  # Add the generated include directory so gotcha_config.h can be found
  if (TARGET gotcha)
    get_target_property(_gotcha_imported gotcha IMPORTED)
    if (NOT _gotcha_imported)
      set_target_properties(gotcha PROPERTIES C_EXTENSIONS ON)
      target_compile_options(gotcha PRIVATE -std=gnu11 -w)
      target_include_directories(gotcha PUBLIC
        $<BUILD_INTERFACE:${gotcha_BINARY_DIR}/include>
      )
    else ()
      message(STATUS "Gotcha imported")
    endif ()
  endif ()
else ()
  message(STATUS "Gotcha: finding only (CMake < 3.14)")
  find_package(gotcha 
               NAMES gotcha GOTCHA
               REQUIRED
  )
endif ()

if (TARGET gotcha::gotcha)
  get_target_property(gotcha_INCLUDE_DIRS gotcha::gotcha INTERFACE_INCLUDE_DIRECTORIES)
  if (NOT gotcha_INCLUDE_DIRS)
    set(gotcha_INCLUDE_DIRS "")
  endif ()
  set(gotcha_LIBRARIES gotcha::gotcha)
  set(gotcha_FOUND TRUE)
elseif (TARGET gotcha)
  get_target_property(gotcha_INCLUDE_DIRS gotcha INTERFACE_INCLUDE_DIRECTORIES)
  if (NOT gotcha_INCLUDE_DIRS)
    set(gotcha_INCLUDE_DIRS "")
  endif ()
  set(gotcha_LIBRARIES gotcha)
  set(gotcha_FOUND TRUE)
elseif (gotcha_LIBRARIES)
  add_library(gotcha::gotcha UNKNOWN IMPORTED)
  set_target_properties(gotcha::gotcha PROPERTIES
    IMPORTED_LOCATION             "${gotcha_LIBRARIES}"
    INTERFACE_INCLUDE_DIRECTORIES "${gotcha_INCLUDE_DIRS}"
  )
  set(gotcha_LIBRARIES gotcha::gotcha)
  set(gotcha_FOUND TRUE)
else ()
  message(FATAL_ERROR "Gotcha: could not find or create target gotcha::gotcha")
endif ()

message(STATUS "gotcha_INCLUDE_DIRS: ${gotcha_INCLUDE_DIRS}")
message(STATUS "gotcha_LIBRARIES: ${gotcha_LIBRARIES}")
mark_as_advanced(gotcha_INCLUDE_DIRS gotcha_INCLUDE_DIR gotcha_LIBRARIES)
